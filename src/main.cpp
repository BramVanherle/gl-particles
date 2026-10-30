#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

#include "Particle.h"
#include "SimulationParams.h"
#include "ComputeShader.h"
#include "RenderShader.h"

const int WINDOW_WIDTH = 1280 * 2;
const int WINDOW_HEIGHT = 720 * 2;
const int NUM_PARTICLES = 10000;

std::vector<Particle> createInitialParticles(int count) {
    std::vector<Particle> particles(count);
    for (auto& p : particles) {
        p.position = {0.0f, 0.0f};
        p.velocity = {0.0f, 0.0f};
        p.color = {1.0f, 1.0f, 1.0f, 1.0f};
        p.mass = 0.0f;
        p.respawn = 1u;
        p.spawnDelay = -1.0f;
    }
    return particles;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle Simulation", nullptr, nullptr);
    if (!window) { std::cerr << "Window creation failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n"; return -1;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    RenderShader renderShader("../shaders/render/basic.vert", "../shaders/render/basic.frag");
    ComputeShader forcesShader("../shaders/compute/forces.comp");
    ComputeShader respawnShader("../shaders/compute/respawn.comp");

    std::vector<Particle> particles = createInitialParticles(NUM_PARTICLES);

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 particles.size() * sizeof(Particle),
                 particles.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, ssbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, mass));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    SimulationParams params;
    float lastFrame = 0.0f;
    int frameIndex = 0;
    bool requestRespawnAll = false;
    int numGroups = (NUM_PARTICLES + 63) / 64;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float currentFrame = (float)glfwGetTime();
        float dt = std::min(currentFrame - lastFrame, 0.008f);
        lastFrame = currentFrame;

        // Keep min/max ranges valid before they are uploaded to the shader.
        params.spawnPosMinX = std::min(params.spawnPosMinX, params.spawnPosMaxX);
        params.spawnPosMinY = std::min(params.spawnPosMinY, params.spawnPosMaxY);
        params.spawnVelMinX = std::min(params.spawnVelMinX, params.spawnVelMaxX);
        params.spawnVelMinY = std::min(params.spawnVelMinY, params.spawnVelMaxY);
        params.spawnMassMin = std::min(params.spawnMassMin, params.spawnMassMax);
        params.spawnBlueMin = std::min(params.spawnBlueMin, params.spawnBlueMax);

        // Pass 1: integrate forces and mark particles that need respawn.
        forcesShader.use();
        forcesShader.setInt("numParticles", NUM_PARTICLES);
        forcesShader.setFloat("dt", dt);
        forcesShader.setFloat("gravity", params.gravity);
        forcesShader.setFloat("windSpeed", params.windSpeed);
        forcesShader.setFloat("windDirection", params.windDirection);
        glDispatchCompute(numGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Pass 2: respawn marked particles (or all particles when reset is requested).
        respawnShader.use();
        respawnShader.setInt("numParticles", NUM_PARTICLES);
        respawnShader.setInt("frameIndex", frameIndex);
        respawnShader.setInt("respawnAll", requestRespawnAll ? 1 : 0);
        respawnShader.setFloat("dt", dt);
        respawnShader.setFloat("spawnDelayDuration", params.spawnDelayDuration);
        respawnShader.setVec2("spawnPosMin", params.spawnPosMinX, params.spawnPosMinY);
        respawnShader.setVec2("spawnPosMax", params.spawnPosMaxX, params.spawnPosMaxY);
        respawnShader.setVec2("spawnVelMin", params.spawnVelMinX, params.spawnVelMinY);
        respawnShader.setVec2("spawnVelMax", params.spawnVelMaxX, params.spawnVelMaxY);
        respawnShader.setFloat("spawnMassMin", params.spawnMassMin);
        respawnShader.setFloat("spawnMassMax", params.spawnMassMax);
        respawnShader.setFloat("spawnBlueMin", params.spawnBlueMin);
        respawnShader.setFloat("spawnBlueMax", params.spawnBlueMax);
        glDispatchCompute(numGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        requestRespawnAll = false;
        frameIndex++;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Particle Simulation");
        ImGui::Text("Particles: %d", NUM_PARTICLES);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::Separator();
        ImGui::SliderFloat("Gravity", &params.gravity, 0.0f, 9.81f);
        ImGui::SliderFloat("Wind Speed", &params.windSpeed, 0.0f, 1.0f);
        ImGui::SliderFloat("Wind Direction", &params.windDirection, 0.0f, 20.0f);

        ImGui::Separator();
        ImGui::Text("Spawn Distribution");
        ImGui::SliderFloat("Pos X Min", &params.spawnPosMinX, 0.0f, 1.0f);
        ImGui::SliderFloat("Pos X Max", &params.spawnPosMaxX, 0.0f, 1.0f);
        ImGui::SliderFloat("Pos Y Min", &params.spawnPosMinY, 0.0f, 1.0f);
        ImGui::SliderFloat("Pos Y Max", &params.spawnPosMaxY, 0.0f, 1.0f);
        if (ImGui::Button("Full Screen Spawn Area")) {
            params.spawnPosMinX = 0.0f;
            params.spawnPosMaxX = 1.0f;
            params.spawnPosMinY = 0.0f;
            params.spawnPosMaxY = 1.0f;
            requestRespawnAll = true;
        }
        ImGui::SliderFloat("Vel X Min", &params.spawnVelMinX, -1.0f, 1.0f);
        ImGui::SliderFloat("Vel X Max", &params.spawnVelMaxX, -1.0f, 1.0f);
        ImGui::SliderFloat("Vel Y Min", &params.spawnVelMinY, -1.0f, 1.0f);
        ImGui::SliderFloat("Vel Y Max", &params.spawnVelMaxY, -1.0f, 1.0f);
        ImGui::SliderFloat("Mass Min", &params.spawnMassMin, 0.01f, 5.0f);
        ImGui::SliderFloat("Mass Max", &params.spawnMassMax, 0.01f, 5.0f);
        ImGui::SliderFloat("Blue Min", &params.spawnBlueMin, 0.0f, 1.0f);
        ImGui::SliderFloat("Blue Max", &params.spawnBlueMax, 0.0f, 1.0f);
        ImGui::SliderFloat("Spawn Delay (seconds)", &params.spawnDelayDuration, 0.0f, 5.0f);

        ImGui::Separator();
        if (ImGui::Button("Reset Simulation")) {
            requestRespawnAll = true;
        }
        ImGui::End();

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderShader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &ssbo);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}