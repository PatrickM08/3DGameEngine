#pragma once
#include "shader_s.h"
#include <cstdint>
#include <vector>
#include <initializer_list>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>


struct Entity {
    uint32_t id;
};

struct TransformComponent {
    glm::mat4 transform;
};

struct SkyboxTag {
    bool isSkybox; //This should change
};

/*
struct MeshComponent {
    GLuint vao;
    GLsizei vertexCount;
    GLsizei instanceCount;

    MeshComponent()
        : vao(0), vertexCount(0), instanceCount(0)
    {
    }

    MeshComponent(GLuint vao, GLsizei vertexCount, GLsizei instanceCount)
        : vao(vao), vertexCount(vertexCount), instanceCount(instanceCount)
    {
    }
};

struct MaterialComponent {
    Shader shader;
    std::vector<GLuint> textureIds;
    std::vector<GLenum> textureBindTargets;

    MaterialComponent()
    {
    }
    MaterialComponent(Shader shader,
        std::initializer_list<GLuint> textureIds,
        std::initializer_list<GLenum> textureBindTargets)
        : shader(shader), textureIds(textureIds), textureBindTargets(textureBindTargets)
    {
    }
};
*/