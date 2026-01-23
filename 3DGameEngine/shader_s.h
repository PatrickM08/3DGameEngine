
#pragma once
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>

inline std::string readShaderFile(const std::string &path) {
    std::string shaderCode;
    std::ifstream shaderFile;

    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shaderFile.open(path);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure &e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n"
                    << "Path: " << path << "\n"
                    << "Error: " << e.what() << std::endl;
    }
    return shaderCode;
}

inline void checkShaderCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout
                << "ERROR::SHADER_COMPILATION_ERROR of type: " << type
                << "\n"
                << infoLog
                << "\n -- "
                    "--------------------------------------------------- -- "
                << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout
                << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog
                << "\n -- "
                    "--------------------------------------------------- -- "
                << std::endl;
        }
    }
}



inline uint32_t createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vp = vertexPath;
    std::string fp = fragmentPath;
#ifdef PROJECT_SOURCE_DIR
    vp = std::string(PROJECT_SOURCE_DIR) + "/" + vertexPath;
    fp = std::string(PROJECT_SOURCE_DIR) + "/" + fragmentPath;
#endif
    std::string vShaderString = readShaderFile(vp);
    std::string fShaderString = readShaderFile(fp);
    //  Can't return const char* directly due to dangling pointer
    const char* vShaderCode = vShaderString.c_str();
    const char* fShaderCode = fShaderString.c_str();

    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkShaderCompileErrors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkShaderCompileErrors(fragment, "FRAGMENT");

    uint32_t id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    checkShaderCompileErrors(id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return id;
}