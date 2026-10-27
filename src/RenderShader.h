#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class RenderShader {
public:
    GLuint program;

    explicit RenderShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
        auto const vertSource = readFile(vertexShaderPath);
        auto const fragSource = readFile(fragmentShaderPath);

        GLuint vert = compileShader(GL_VERTEX_SHADER, vertSource.c_str());
        GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSource.c_str());

        program = glCreateProgram();
        glAttachShader(program, vert);
        glAttachShader(program, frag);
        glLinkProgram(program);
        glDeleteShader(vert);
        glDeleteShader(frag);

    }

    void use() const { glUseProgram(program); }

    ~RenderShader() { glDeleteProgram(program); }

private:
    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open compute shader: " << path << std::endl;
            return {};
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    static GLuint compileShader(GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Shader error: " << log << std::endl;
        }
        return shader;
    }
};