#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "entity.h"
#include "asset_manager.h"
#include <unordered_map>

constexpr int MAX_ENTITIES = 100;

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
        materials.emplace("cube", MaterialComponent(cube.shader, { cube.textures[0].id, cube.textures[1].id }, { cube.textures[0].target, cube.textures[1].target }));

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