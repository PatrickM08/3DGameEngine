#include "ecs.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "shader_s.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include "camera.h"


ComponentBlob::ComponentBlob(ComponentBlob&& other) noexcept
    : typeInfo(other.typeInfo), size(other.size), data(std::move(other.data)) {
}

ECS::ECS() {
    entityCount = 0;
    entityTemplates.reserve(10);
    parseEntityTemplateFile();
    parseSceneFile();
}

void ECS::parseEntityTemplateFile() {
    std::ifstream file("entities.txt");
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
        else if (prefix == "renderable") {
            entityTemplates[entityName].components.emplace_back(RenderableTag{});
        }
        else if (prefix == "transform") {
            TransformComponent transform{ glm::mat4(1.0f), glm::quat_cast(glm::mat4(1.0f)) };
            entityTemplates[entityName].components.emplace_back(transform);
        }
        else if (prefix == "skybox") {
            entityTemplates[entityName].components.emplace_back(SkyboxTag{});
        }
        else if (prefix == "velocity") {
            VelocityComponent velocity{ glm::vec3(0.0f, 0.0f, 0.0f) };
            entityTemplates[entityName].components.emplace_back(velocity);
        }
        else if (prefix == "speed") {
            float sp;
            stream >> sp;
            SpeedComponent speed{ sp };
            entityTemplates[entityName].components.emplace_back(speed);
        }
        else if (prefix == "rotationSpeed") {
            float rs;
            stream >> rs;
            RotationSpeedComponent rotationSpeed{ rs };
            entityTemplates[entityName].components.emplace_back(rotationSpeed);
        }
    }
}

void ECS::parseSceneFile() {
    std::ifstream file("scene.txt");
    if (!file.is_open()) {
        throw std::runtime_error("Error opening scene file.");
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string prefix;
        stream >> prefix;
        if (prefix == "pointlight") {
            glm::vec3 pos;
            stream >> pos.x >> pos.y >> pos.z;
            pointLightPositions.push_back(pos);
        }
        else if (prefix == "entity") {
            entityCount++;
            std::string entityName;
            stream >> entityName;
            EntityTemplate& entityTemplate = entityTemplates[entityName];
            for (auto& blob : entityTemplate.components) {
                if (*blob.typeInfo == typeid(MeshHandleStorage)) {
                    auto& meshHandleStorage = deserializeBlob<MeshHandleStorage>(blob);
                    meshSet.add(entityCount, assetManager.getMesh(meshHandleStorage.meshHandle));
                }
                else if (*blob.typeInfo == typeid(MaterialHandleStorage)) {
                    auto& materialHandleStorage = deserializeBlob<MaterialHandleStorage>(blob);
                    MaterialData& material = assetManager.getMaterial(materialHandleStorage.materialHandle);
                    material.shader.use();
                    for (int i = 0; i < material.textures.size(); i++) {
                        material.shader.setIntUniform("texture" + std::to_string(i), i);
                    }
                    material.shader.setFloatUniform("shininess", material.shininess);
                    material.shader.setMat3Uniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(glm::mat4(1.0f)))));
                    material.shader.setIntUniform("numberOfPointLights", pointLightPositions.size());

                    for (int i = 0; i < pointLightPositions.size(); i++) {
                        std::string index = std::to_string(i);
                        material.shader.setVec3Uniform("pointLights[" + index + "].position", pointLightPositions[i]);
                        material.shader.setVec3Uniform("pointLights[" + index + "].ambient", material.ambient);
                        material.shader.setVec3Uniform("pointLights[" + index + "].diffuse", material.diffuse);
                        material.shader.setVec3Uniform("pointLights[" + index + "].specular", material.specular);
                        material.shader.setFloatUniform("pointLights[" + index + "].constant", 1.0f);
                        material.shader.setFloatUniform("pointLights[" + index + "].linear", 0.09f);
                        material.shader.setFloatUniform("pointLights[" + index + "].quadratic", 0.032f);
                    }

                    materialSet.add(entityCount, material);
                }
                else if (*blob.typeInfo == typeid(RenderableTag)) {
                    renderableSet.add(entityCount, RenderableTag{});
                }
                else if (*blob.typeInfo == typeid(TransformComponent)) {
                    auto& transformComponent = deserializeBlob<TransformComponent>(blob);
                    transformSet.add(entityCount, transformComponent);
                }
                else if (*blob.typeInfo == typeid(SkyboxTag)) {
                    auto& skyboxTag = deserializeBlob<SkyboxTag>(blob);
                    skyboxSet.add(entityCount, skyboxTag);
                }
                else if (*blob.typeInfo == typeid(VelocityComponent)) {
                    auto& velocity = deserializeBlob<VelocityComponent>(blob);
                    velocitySet.add(entityCount, velocity);
                }
                else if (*blob.typeInfo == typeid(SpeedComponent)) {
                    auto& speed = deserializeBlob<SpeedComponent>(blob);
                    speedSet.add(entityCount, speed);
                }
                else if (*blob.typeInfo == typeid(RotationSpeedComponent)) {
                    auto& rotSpeed = deserializeBlob<RotationSpeedComponent>(blob);
                    rotationSpeedSet.add(entityCount, rotSpeed);
                }
            }
        }
        else if (prefix == "pos") {
            glm::vec3 spawnPosition;
            stream >> spawnPosition.x >> spawnPosition.y >> spawnPosition.z;
            if (!transformSet.hasComponent(entityCount)) {
                transformSet.add(entityCount, TransformComponent{});
            }
            auto& transform = transformSet.getComponent(entityCount);
            transform.transform = glm::translate(transform.transform, spawnPosition);
        }
        else if (prefix == "playerInputWorld") {
            inputWorldSet.add(entityCount, PlayerInputWorldTag{});
        }
        else if (prefix == "playerInputTank") {
            inputTankSet.add(entityCount, PlayerInputTankTag{});
        }
        else if (prefix == "noClip") {
            inputNoClipSet.add(entityCount, PlayerInputNoClipTag{});
        }
        else if (prefix == "patrol") {
            PatrolComponent patrol;
            stream >> patrol.direction.x >> patrol.direction.y >> patrol.direction.z >> patrol.magnitude;
            patrol.direction = glm::normalize(patrol.direction);
            patrolSet.add(entityCount, patrol);
        }
        else if (prefix == "collision") {
            CollisionComponent collision;
            stream >> collision.minX >> collision.maxX
                >> collision.minY >> collision.maxY
                >> collision.minZ >> collision.maxZ;
            collisionSet.add(entityCount, collision);
        }
        else if (prefix == "camera") {
            std::string type;
            std::string restOfLine;
            std::getline(stream, restOfLine);
            std::replace(restOfLine.begin(), restOfLine.end(), ',', ' ');
            std::istringstream cameraStream(restOfLine);
            CameraComponent camera;
            cameraStream >> type >> camera.positionOffset.x >> camera.positionOffset.y >> camera.positionOffset.z >> camera.worldUp.x
                >> camera.worldUp.y >> camera.worldUp.z >> camera.yaw >> camera.pitch;
            camera.position = glm::vec3(transformSet.getComponent(entityCount).transform[3]) + camera.positionOffset;
            CameraType cameraType = CameraType::FIXED;
            if (type == "mouse") cameraType = CameraType::MOUSETURN;
            else if (type == "fixed") cameraType = CameraType::FIXED;
            camera.cameraType = cameraType;
            updateCameraVectors(camera);
            cameraSet.add(entityCount, camera);
        }
        else if (prefix == "algo") {
            std::string algorithmName;
            stream >> algorithmName;
            if (algorithmName == "grid") {
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

                glBindVertexArray(meshSet.getComponent(entityCount).vao);
                GLuint instanceVBO;
                glGenBuffers(1, &instanceVBO);
                glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_BOXES, translations, GL_STATIC_DRAW);
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(3);
                glVertexAttribDivisor(3, 1);

                instancedSet.add(entityCount, InstancedComponent{ NUM_BOXES });
            }
        }
    }
}