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
unsigned int loadTexture(char const* path);

constexpr int MAX_ENTITIES = 100;

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
    GLenum drawMode;
    GLsizei vertexCount;
    GLsizei instanceCount;

    MeshComponent()
        : vao(0), drawMode(0), vertexCount(0), instanceCount(0)
    {
    }

    MeshComponent(GLuint vao, GLenum drawMode, GLsizei vertexCount, GLsizei instanceCount) 
        : vao(vao), drawMode(drawMode), vertexCount(vertexCount), instanceCount(instanceCount)
    {
    }
};

struct MaterialComponent {
    std::shared_ptr<Shader> shader;
    std::vector<GLuint> textureIds;
    std::vector<GLenum> textureBindTargets;

    MaterialComponent() 
        : shader(nullptr)
    {
    }
    MaterialComponent(std::shared_ptr<Shader> shader,
        std::initializer_list<GLuint> textureIds,
        std::initializer_list<GLenum> textureBindTargets)
        : shader(shader), textureIds(textureIds), textureBindTargets(textureBindTargets)
    {
    }
    void addTextureIdAndBindTarget(GLuint textureId, GLenum textureBindTarget) {
        textureIds.push_back(textureId);
        textureBindTargets.push_back(textureBindTarget);
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
        materials.emplace("skybox", MaterialComponent(skyboxShader, { skyboxTextureId }, { GL_TEXTURE_CUBE_MAP }));

        std::shared_ptr<Shader> cubeShader = std::make_shared<Shader>("cube.vs", "cube.fs");
        GLuint containerDiffuseId = loadTexture("container2.png");
        GLuint containerSpecularId = loadTexture("container2_specular.png");
        cubeShader->use();
        cubeShader->setIntUniform("material.diffuse", 0);
        cubeShader->setIntUniform("material.specular", 1);
        cubeShader->setFloatUniform("material.shininess", 32.0f);

        // This shouldn't be here.
        glm::vec3 lightPos = glm::vec3(0.0f, 1.0f, 0.0f);
        cubeShader->setVec3Uniform("light.position", lightPos);
        cubeShader->setVec3Uniform("light.ambient", 0.2f, 0.2f, 0.2f);
        cubeShader->setVec3Uniform("light.diffuse", 0.5f, 0.5f, 0.5f);
        cubeShader->setVec3Uniform("light.specular", 1.0f, 1.0f, 1.0f);
        materials.emplace("cube", MaterialComponent(cubeShader, { containerDiffuseId, containerSpecularId }, { GL_TEXTURE_2D }));
    }
};

struct Meshes {
    std::unordered_map<std::string, MeshComponent> meshes;

    Meshes() {
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
        GLuint skyboxVAO, skyboxVBO;
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        MeshComponent skyboxMesh(skyboxVAO, GL_TRIANGLES, 36, 1);
        meshes.emplace("skybox", skyboxMesh);


        float vertices[] = {
            // Back face (-Z)
            -0.5f, -0.5f, -0.5f,  0, 0, -1,  0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0, 0, -1,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0, 0, -1,  1.0f, 0.0f,

             0.5f,  0.5f, -0.5f,  0, 0, -1,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0, 0, -1,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0, 0, -1,  0.0f, 1.0f,

            // Front face (+Z)
            -0.5f, -0.5f,  0.5f,  0, 0, 1,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0, 0, 1,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0, 0, 1,  1.0f, 1.0f,

             0.5f,  0.5f,  0.5f,  0, 0, 1,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0, 0, 1,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0, 0, 1,  0.0f, 0.0f,

            // Left face (-X)
            -0.5f,  0.5f,  0.5f,  -1, 0, 0,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1, 0, 0,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1, 0, 0,  0.0f, 1.0f,

            -0.5f, -0.5f, -0.5f, -1, 0, 0,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1, 0, 0,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1, 0, 0,  1.0f, 0.0f,

            // Right face (+X)
             0.5f,  0.5f,  0.5f,  1, 0, 0,  1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1, 0, 0,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1, 0, 0,  1.0f, 1.0f,

             0.5f, -0.5f, -0.5f,  1, 0, 0,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1, 0, 0,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1, 0, 0,  0.0f, 0.0f,

             // Bottom face (-Y)
             -0.5f, -0.5f, -0.5f,  0, -1, 0,  0.0f, 1.0f,
              0.5f, -0.5f, -0.5f,  0, -1, 0,  1.0f, 1.0f,
              0.5f, -0.5f,  0.5f,  0, -1, 0,  1.0f, 0.0f,

              0.5f, -0.5f,  0.5f,  0, -1, 0,  1.0f, 0.0f,
             -0.5f, -0.5f,  0.5f,  0, -1, 0,  0.0f, 0.0f,
             -0.5f, -0.5f, -0.5f,  0, -1, 0,  0.0f, 1.0f,

             // Top face (+Y)
             -0.5f,  0.5f, -0.5f,  0, 1, 0,  0.0f, 1.0f,
              0.5f,  0.5f,  0.5f,  0, 1, 0,  1.0f, 0.0f,
              0.5f,  0.5f, -0.5f,  0, 1, 0,  1.0f, 1.0f,

              0.5f,  0.5f,  0.5f,  0, 1, 0,  1.0f, 0.0f,
             -0.5f,  0.5f, -0.5f,  0, 1, 0,  0.0f, 1.0f,
             -0.5f,  0.5f,  0.5f,  0, 1, 0,  0.0f, 0.0f
        };

        GLuint cubeVAO, cubeVBO, cubeInstanceVBO;
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
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

        glGenBuffers(1, &cubeInstanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_BOXES, translations, GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);
        MeshComponent cubeMesh(cubeVAO, GL_TRIANGLES, 36, NUM_BOXES);
        meshes.emplace("cube", cubeMesh);
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
    RenderSystem() {
        initOpenglState();
    }

    void renderScene(Camera& camera, ECS& scene) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (Entity e : scene.entitiesInScene) {
            MaterialComponent& material = scene.materialsInScene[e.id];
            MeshComponent& mesh = scene.meshesInScene[e.id];
            TransformComponent& transform = scene.transformsInScene[e.id];
            material.shader->use();
            material.shader->setMat4Uniform("projection", camera.projectionMatrix);
            if (scene.skyboxesInScene[e.id].isSkybox) {
                glm::mat4 skyboxViewMatrix(glm::mat3(camera.viewMatrix));
                material.shader->setMat4Uniform("view", skyboxViewMatrix);
                glDepthFunc(GL_LEQUAL);
            }
            else {
                material.shader->setMat4Uniform("view", camera.viewMatrix);
                material.shader->setMat4Uniform("model", transform.transform);
                material.shader->setVec3Uniform("cameraPos", camera.Position);
            }
            glBindVertexArray(mesh.vao);
            for (int i = 0; i < material.textureIds.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(material.textureBindTargets[i], material.textureIds[i]);
            }
            if (mesh.instanceCount == 1) {
                glDrawArrays(mesh.drawMode, 0, mesh.vertexCount);
            }
            else {
                glDrawArraysInstanced(mesh.drawMode, 0, mesh.vertexCount, mesh.instanceCount);
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