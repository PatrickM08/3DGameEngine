
#pragma once
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

struct Shader {
    uint32_t id;
    std::unordered_map<std::string, GLint> uniforms;
    // TODO: SORT THIS OUT
    static const bool UNIFORM_DEBUG = false;

    Shader() : id(0) {}

    Shader(const std::string &vertexPath, const std::string &fragmentPath)
    {
        std::string vp = vertexPath;
        std::string fp = fragmentPath;
#ifdef PROJECT_SOURCE_DIR
        vp = std::string(PROJECT_SOURCE_DIR) + "/" + vertexPath;
        fp = std::string(PROJECT_SOURCE_DIR) + "/" + fragmentPath;
#endif
        std::string vShaderString = readShaderFile(vp);
        std::string fShaderString = readShaderFile(fp);
        //  Can't return const char* directly due to dangling pointer
        const char *vShaderCode = vShaderString.c_str();
        const char *fShaderCode = fShaderString.c_str();

        unsigned int vertex, fragment;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkShaderCompileErrors(vertex, "VERTEX");

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkShaderCompileErrors(fragment, "FRAGMENT");

        id = glCreateProgram();
        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        glLinkProgram(id);
        checkShaderCompileErrors(id, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    std::string readShaderFile(const std::string &path)
    {
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

    void checkShaderCompileErrors(GLuint shader, std::string type)
    {
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

    void use() const { glUseProgram(id); }

    void setBoolUniform(const std::string &name, bool value)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform1i(it->second, (int)value);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform1i(location, (int)value);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setIntUniform(const std::string &name, int value)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform1i(it->second, value);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform1i(location, value);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setFloatUniform(const std::string &name, float value)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform1f(it->second, value);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform1f(location, value);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setVec2Uniform(const std::string &name, const glm::vec2 &value)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform2fv(it->second, 1, &value[0]);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform2fv(location, 1, &value[0]);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setVec2Uniform(const std::string &name, float x, float y)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform2f(it->second, x, y);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform2f(location, x, y);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setVec3Uniform(const std::string &name, const glm::vec3 &value)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform3fv(it->second, 1, &value[0]);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform3fv(location, 1, &value[0]);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setVec3Uniform(const std::string &name, float x, float y, float z)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform3f(it->second, x, y, z);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform3f(location, x, y, z);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setVec4Uniform(const std::string &name, const glm::vec4 &value)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform4fv(it->second, 1, &value[0]);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform4fv(location, 1, &value[0]);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setVec4Uniform(const std::string &name, float x, float y, float z,
                        float w)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniform4f(it->second, x, y, z, w);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniform4f(location, x, y, z, w);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setMat2Uniform(const std::string &name, const glm::mat2 &mat)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniformMatrix2fv(it->second, 1, GL_FALSE, &mat[0][0]);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setMat3Uniform(const std::string &name, const glm::mat3 &mat)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniformMatrix3fv(it->second, 1, GL_FALSE, &mat[0][0]);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }

    void setMat4Uniform(const std::string &name, const glm::mat4 &mat)
    {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) {
            glUniformMatrix4fv(it->second, 1, GL_FALSE, &mat[0][0]);
            return;
        }
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location != -1) {
            glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
            uniforms.emplace(name, location);
            return;
        }
        if (UNIFORM_DEBUG)
        std::cout << "Error finding uniform: " << name << std::endl;
    }
};
