#pragma once


#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ComputeShader {
public:
    GLuint program;

    explicit ComputeShader(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open compute shader: " << path << std::endl;
            return;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        std::string src = ss.str();
        const char* csrc = src.c_str();

        GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader, 1, &csrc, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Compute shader error (" << path << "): " << log << std::endl;
        }

        program = glCreateProgram();
        glAttachShader(program, shader);
        glLinkProgram(program);
        glDeleteShader(shader);
    }

    void use() { glUseProgram(program); }

    void setInt(const std::string& name, int val) {
        glUniform1i(glGetUniformLocation(program, name.c_str()), val);
    }
    void setFloat(const std::string& name, float val) {
        glUniform1f(glGetUniformLocation(program, name.c_str()), val);
    }
    void setVec2(const std::string& name, float x, float y) {
        glUniform2f(glGetUniformLocation(program, name.c_str()), x, y);
    }

    ~ComputeShader() { glDeleteProgram(program); }
};
