#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "entity.h"
#include "asset_manager.h"
#include <unordered_map>
#include <memory>


struct MeshHandleStorage {
    uint32_t meshHandle;
};

struct MaterialHandleStorage {
    uint32_t materialHandle;
};

constexpr int MAX_ENTITIES = 100;

uint32_t nextComponentID = 0;

template<typename T>
uint32_t getTypeID() {
    static uint32_t id = nextComponentID++;
    return id;
}

struct ComponentBlob {
    uint32_t typeID;
    size_t size;
    std::unique_ptr<uint8_t[]> data;

    template<typename T>
    ComponentBlob(const T& component) {
        typeID = getTypeID<T>();
        size = sizeof(T);
        data = std::make_unique<uint8_t[]>(sizeof(T));
        std::memcpy(data.get(), &component, size);
    }

    ComponentBlob(ComponentBlob&& other) noexcept
        : typeID(other.typeID), size(other.size), data(std::move(other.data)) {
    }

    // Delete copy operations, this type cannot be copied
    ComponentBlob(const ComponentBlob&) = delete;
    ComponentBlob& operator=(const ComponentBlob&) = delete;
};

template<typename T>
T& deserializeBlob(ComponentBlob& blob) {
    if (blob.size != sizeof(T)) {
        throw std::runtime_error("Size mismatch!");
    }
    return *reinterpret_cast<T*>(blob.data.get());
}

// We read entities.txt and create the entity templates, when we create the scene we retrieve and deserialize and create
// actual ecs entities.
struct EntityTemplate {
    std::string name;
    std::vector<ComponentBlob> components; // Maybe this can be replaced if we know the maximum number of components
};

struct Materials {
    std::unordered_map<std::string, MaterialData> materials;

    Materials(AssetManager& assetManager) {
        MaterialData skybox = assetManager.getMaterial(2);
        skybox.shader.use();
        skybox.shader.setIntUniform("skybox", 0);
        materials.emplace("skybox", skybox);

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
        materials.emplace("cube", cube);

        MaterialData simpleMaterial = assetManager.getMaterial(0);
        materials.emplace("simple", simpleMaterial);
    }
};

struct Meshes {
    std::unordered_map<std::string, MeshData> meshes;

    Meshes(AssetManager& assetManager) {
        MeshData skybox = assetManager.getMesh(2);
        meshes.emplace("skybox", skybox);


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
        meshes.emplace("cube", cube);

        MeshData baseplate = assetManager.getMesh(0);
        meshes.emplace("baseplate", baseplate);
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
        entityTemplates.reserve(5);
        parseEntityTemplateFile();
    }
    void updateTransforms(uint32_t entityId, glm::mat4 transform) {
        transformsInScene[entityId].transform = transform;
    }

    void parseEntityTemplateFile() {
        std::ifstream file("entities.txt"); // CHANGE THIS SHOULDNT BE HERE
        if (!file.is_open()) {
            throw std::runtime_error("Error opening entity template file.");
        }
        std::string line;
        std::string entityName;
        while (std::getline(file, line)) {
            std::istringstream stream(line);
            std::string prefix;
            stream >> prefix;
            if (prefix == "entity") {
                stream >> entityName;
                entityTemplates[entityName] = EntityTemplate();
                entityTemplates[entityName].name = entityName;
            }
            else if (prefix == "mesh") {
                uint32_t meshHandle;
                stream >> meshHandle;
                MeshHandleStorage handle{ meshHandle };
                entityTemplates[entityName].components.emplace_back(handle);
            }
            else if (prefix == "material") {
                uint32_t materialHandle;
                stream >> materialHandle;
                MaterialHandleStorage handle{ materialHandle };
                entityTemplates[entityName].components.emplace_back(handle);
            }
            else if (prefix == "transform") {
                TransformComponent transform{ glm::mat4(1.0f) }; //This needs to be changed, or not could keep everything here until scene file
                entityTemplates[entityName].components.emplace_back(transform);
            }
            else if (prefix == "skybox") {
                SkyboxTag skyboxTag{ true };
                entityTemplates[entityName].components.emplace_back(skyboxTag);
            }
        }
    }

    void parseSceneFile() {
        std::ifstream file("scene.txt"); //CHANGE
        if (!file.is_open()) {
            throw std::runtime_error("Error opening scene file.");
        }
        std::string line;
        uint32_t entityCount = 0;
        while (std::getline(file, line)) {
            std::istringstream stream(line);
            std::string prefix;
            stream >> prefix;
            if (prefix == "pointlight") {
                glm::vec3 pos;
                stream >> pos.x >> pos.y >> pos.z;
                pointlightPostions.push_back(pos);
            }
            if (prefix == "entity") {
                entitiesInScene.emplace_back(entityCount++);
                auto& entity = entitiesInScene.back();
                std::string entityName;
                stream >> entityName;

                for (auto& blob : entityTemplate.components) {
                    if (blob.typeID == getTypeID<MeshHandleStorage>()) {
                        auto& meshHandleStorage = deserializeBlob<MeshHandleStorage>(blob);
                        meshesInScene[entity.id] = assetManager.getMesh(meshHandleStorage.meshHandle);
                    }
                    else if (blob.typeID == getTypeID<MaterialHandleStorage>()) {
                        auto& materialHandleStorage = deserializeBlob<MaterialHandleStorage>(blob);
                        materialsInScene[entity.id] = assetManager.getMaterial(materialHandleStorage.materialHandle);
                    }
                    else if (blob.typeID == getTypeID<TransformComponent>()) {
                        auto& transformComponent = deserializeBlob<TransformComponent>(blob);
                        transformsInScene[entity.id] = transformComponent;
                    }
                    else if (blob.typeID == getTypeID<SkyboxTag>()) {
                        auto& skyboxTag = deserializeBlob<SkyboxTag>(blob);
                        skyboxesInScene[entity.id] = skyboxTag;
                    }
                }
            }
        }
    }

public:
    std::vector<TransformComponent> transformsInScene;
    std::vector<MeshData> meshesInScene;
    std::vector<MaterialData> materialsInScene;
    std::vector<Entity> entitiesInScene;
    std::vector<SkyboxTag> skyboxesInScene;
    std::unordered_map<std::string, EntityTemplate> entityTemplates;
    std::vector<glm::vec3> pointlightPostions;
};




