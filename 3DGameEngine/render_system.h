#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "shader_s.h"
#include "stb_image.h"
#include <memory>
#include <unordered_map>
#include "camera.h"
#include "window.h"
#include "asset_manager.h"

GLuint loadCubemap(const char** faces, const int numberOfFaces);
unsigned int loadTexture(char const* path);

constexpr int MAX_ENTITIES = 100;

struct Framebuffer {
    GLuint buffer;
    GLuint textureAttachment;
    GLuint renderBufferObject;
    uint32_t width, height;
};

Framebuffer createFrameBuffer(uint32_t width, uint32_t height);

struct Entity {
    uint32_t id;
};

struct TransformComponent {
    glm::mat4 transform;
};

struct SkyboxTag {
    bool isSkybox;
};

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

struct Materials {
    std::unordered_map<std::string, MaterialComponent> materials;

    Materials(AssetManager& assetManager) {
        MaterialData skybox = assetManager.getMaterial(2);
        skybox.shader.use();
        skybox.shader.setIntUniform("skybox", 0);
        materials.emplace("skybox", MaterialComponent(skybox.shader, { skybox.textures[0].id }, { skybox.textures[0].target }));

        MaterialData cube = assetManager.getMaterial(1);
        cube.shader.use();
        cube.shader.setIntUniform("material.diffuse", 0);
        cube.shader.setIntUniform("material.specular", 1);
        cube.shader.setFloatUniform("material.shininess", 32.0f);

        // This shouldn't be here.
        cube.shader.setIntUniform("numberOfPointLights", 2);
        glm::vec3 lightPositions[2] = { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(20.0f, 1.0f, -20.0f) };
        for (int i = 0; i < 2; i++) {
            std::string index = std::to_string(i);
            cube.shader.setVec3Uniform("pointLights[" + index + "].position", lightPositions[i]);
            cube.shader.setVec3Uniform("pointLights[" + index + "].ambient", 0.2f, 0.2f, 0.2f);
            cube.shader.setVec3Uniform("pointLights[" + index + "].diffuse", 0.5f, 0.5f, 0.5f);
            cube.shader.setVec3Uniform("pointLights[" + index + "].specular", 1.0f, 1.0f, 1.0f);
            cube.shader.setFloatUniform("pointLights[" + index + "].constant", 1.0f);
            cube.shader.setFloatUniform("pointLights[" + index + "].linear", 0.09f);
            cube.shader.setFloatUniform("pointLights[" + index + "].quadratic", 0.032f);
        }
        materials.emplace("cube", MaterialComponent(cube.shader, { cube.textures[0].id, cube.textures[1].id}, { cube.textures[0].target, cube.textures[1].target }));

        MaterialData simpleMaterial = assetManager.getMaterial(0);
        materials.emplace("simple", MaterialComponent(simpleMaterial.shader, {}, {}));
    }
};

struct Meshes {
    std::unordered_map<std::string, MeshComponent> meshes;

    Meshes(AssetManager& assetManager) {
        MeshData skybox = assetManager.getMesh(2);
        MeshComponent skyboxMesh(skybox.vao, skybox.vertexCount, 1);
        meshes.emplace("skybox", skyboxMesh);


        const int NUM_BOXES = 1024;
        int index = 0;
        glm::vec3 translations[NUM_BOXES];
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                translations[index].x = i * 2;
                translations[index].y = 0;
                translations[index].z = -j * 2;
                index++;
            }
        }

        MeshData cube = assetManager.getMesh(1);
        glBindVertexArray(cube.vao);
        GLuint cubeInstanceVBO;
        glGenBuffers(1, &cubeInstanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_BOXES, translations, GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);
        MeshComponent cubeMesh(cube.vao, cube.vertexCount, NUM_BOXES);
        meshes.emplace("cube", cubeMesh);

        MeshData baseplate = assetManager.getMesh(0);
        MeshComponent baseplateMesh(baseplate.vao, baseplate.vertexCount, 1);
        meshes.emplace("baseplate", baseplateMesh);
    }
};


class ECS {
public:
    ECS() {
        transformsInScene.resize(MAX_ENTITIES);
        meshesInScene.resize(MAX_ENTITIES);
        materialsInScene.resize(MAX_ENTITIES);
        entitiesInScene.reserve(MAX_ENTITIES);
        skyboxesInScene.resize(MAX_ENTITIES);
    }
    void updateTransforms(uint32_t entityId, glm::mat4 transform) {
        transformsInScene[entityId].transform = transform;
    }
public:
    std::vector<TransformComponent> transformsInScene;
    std::vector<MeshComponent> meshesInScene;
    std::vector<MaterialComponent> materialsInScene;
    std::vector<Entity> entitiesInScene;
    std::vector<SkyboxTag> skyboxesInScene;
};


class RenderSystem {
public:
    RenderSystem(Window& window) {
        initOpenglState();
        Framebuffer framebuffer = createFrameBuffer(window.width, window.height);
    }

    void renderScene(Camera& camera, ECS& scene) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (Entity e : scene.entitiesInScene) {
            MaterialComponent& material = scene.materialsInScene[e.id];
            MeshComponent& mesh = scene.meshesInScene[e.id];
            TransformComponent& transform = scene.transformsInScene[e.id];
            material.shader.use();
            material.shader.setMat4Uniform("projection", camera.projectionMatrix);
            if (scene.skyboxesInScene[e.id].isSkybox) {
                glm::mat4 skyboxViewMatrix(glm::mat3(camera.viewMatrix));
                material.shader.setMat4Uniform("view", skyboxViewMatrix);
                glDepthFunc(GL_LEQUAL);
            }
            else {
                material.shader.setMat4Uniform("view", camera.viewMatrix);
                material.shader.setMat4Uniform("model", transform.transform);
                material.shader.setVec3Uniform("cameraPos", camera.Position);
            }
            glBindVertexArray(mesh.vao);
            for (int i = 0; i < material.textureIds.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(material.textureBindTargets[i], material.textureIds[i]);
            }
            if (mesh.instanceCount == 1) {
                glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
            }
            else {
                glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.vertexCount, mesh.instanceCount);
            }
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

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

Framebuffer createFrameBuffer(uint32_t width, uint32_t height) {
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return Framebuffer{
        .buffer = framebuffer,
        .textureAttachment = textureColorbuffer,
        .renderBufferObject = rbo
    };
}