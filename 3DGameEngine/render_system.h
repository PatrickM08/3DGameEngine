#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include "shader_s.h"
#include "stb_image.h"
#include <memory>
#include <unordered_map>
#include "camera.h"

GLuint loadCubemap(const char** faces, const int numberOfFaces);

constexpr int MAX_ENTITIES = 1000;

struct Entity {
    uint32_t id;
};

struct TransformComponent {
    glm::mat4 transform;
};

struct MeshComponent {
    GLuint vao;
    GLenum drawMode;
    GLsizei vertexCount;

    MeshComponent()
        : vao(0), drawMode(0), vertexCount(0)
    {
    }

    MeshComponent(GLuint vao, GLenum drawMode, GLsizei vertexCount) 
        : vao(vao), drawMode(drawMode), vertexCount(vertexCount)
    {
    }
};

struct MaterialComponent {
    std::shared_ptr<Shader> shader;
    GLuint textureId;

    MaterialComponent() 
        : shader(nullptr), textureId(0)
    {
    }

    MaterialComponent(std::shared_ptr<Shader> shader, GLuint textureId) :
        shader(shader), textureId(textureId)
    {
    }
};

struct Materials {
    std::unordered_map<std::string, MaterialComponent> materials;

    Materials() {
        std::shared_ptr<Shader> skyboxShader = std::make_shared<Shader>("skybox_vshader.vs", "skybox_fshader.fs");
        const char* skyboxFaces[] = {
        "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
        };
        const int NUM_SKYBOX_FACES = 6;
        GLuint skyboxTextureId = loadCubemap(skyboxFaces, NUM_SKYBOX_FACES);
        skyboxShader->use();
        skyboxShader->setIntUniform("skybox", 0);
        materials.emplace("skybox", MaterialComponent(skyboxShader, skyboxTextureId));
    }
};

struct Meshes {
    std::unordered_map<std::string, MeshComponent> meshes;

    Meshes() {
        GLuint skyboxVAO, skyboxVBO;
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        MeshComponent skyboxMesh(skyboxVAO, GL_TRIANGLES, 36);
        meshes.emplace("skybox", skyboxMesh);
    }
};


class ECS {
public:
    ECS() {
        transformsInScene.resize(MAX_ENTITIES);
        meshesInScene.resize(MAX_ENTITIES);
        materialsInScene.resize(MAX_ENTITIES);
        entitiesInScene.reserve(MAX_ENTITIES);
    }
    void updateTransforms(uint32_t entityId, glm::mat4 transform) {
        transformsInScene[entityId].transform = transform;
    }
public:
    std::vector<TransformComponent> transformsInScene;
    std::vector<MeshComponent> meshesInScene;
    std::vector<MaterialComponent> materialsInScene;
    std::vector<Entity> entitiesInScene;
};


class RenderSystem {
public:
    RenderSystem() {
        initOpenglState();
    }

    void renderScene(Camera& camera, ECS& scene) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (Entity e : scene.entitiesInScene) {
            scene.materialsInScene[e.id].shader->use();
            scene.materialsInScene[e.id].shader->setIntUniform("skybox", 0);
            scene.materialsInScene[e.id].shader->setMat4Uniform("projection", camera.projectionMatrix);
            glm::mat4 skyboxViewMatrix(glm::mat3(camera.viewMatrix));
            scene.materialsInScene[e.id].shader->setMat4Uniform("view", skyboxViewMatrix);
            glBindVertexArray(scene.meshesInScene[e.id].vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, scene.materialsInScene[e.id].textureId);
            glDepthFunc(GL_LEQUAL);
            glDrawArrays(scene.meshesInScene[e.id].drawMode, 0, scene.meshesInScene[e.id].vertexCount);
            glDepthFunc(GL_LESS);
        }
    }
private:
    void initOpenglState() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    }
};


GLuint loadCubemap(const char** faces, const int numberOfFaces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < numberOfFaces; i++)
    {
        unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}