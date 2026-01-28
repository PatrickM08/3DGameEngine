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

ECS::ECS() {
    entityCount = 0;
    visibleEntities.reserve(2000000); // The maximum number of entities that can be rendered per pass.
    visiblePointLights.reserve(256); // The maximum number of lights that can influence the view frustum.
    lightSSBO = createLightSSBO();
    skyboxData.cubemapHandle = assetManager.loadSkyboxCubemap();
    skyboxData.shaderID = createShaderProgram("skybox.vs", "skybox.fs");
    skyboxData.meshVAO = assetManager.meshes[2].vao; // TODO: CHANGE
    sceneUBO = createSceneUBO();
}

void init(ECS& scene) {
    uint32_t entityCount = 0;

    scene.meshSet.add(entityCount, scene.assetManager.meshes[1]);
    scene.velocitySet.add(entityCount, VelocityComponent{glm::vec3(0.0f)});
    scene.speedSet.add(entityCount, SpeedComponent{5.0f});
    scene.transformSet.add(entityCount, TransformComponent{.position = glm::vec3(0.0f, 0.0f, 0.0f)});
    scene.inputNoClipSet.add(entityCount, PlayerInputNoClipTag{});

    CameraComponent camera{
        .positionOffset = glm::vec3(0.0f),
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
        .worldUp = glm::vec3(0, 1, 0),
        .yaw = -45.0f,
        .pitch = -90.0f,
        .cameraType = CameraType::MOUSETURN};

    updateCameraVectors(camera);
    scene.cameraSet.add(entityCount, camera);

    ++entityCount;
    scene.meshSet.add(entityCount, scene.assetManager.meshes[0]);
    scene.materialSet.add(entityCount, scene.assetManager.materials[1]);
    scene.renderableSet.add(entityCount, RenderableTag{});
    scene.transformSet.add(entityCount, TransformComponent{.position = glm::vec3(-20, 0, 20)});

    const uint32_t numEntities = 5000;
    const uint32_t lightFrequency = 50;

    for (uint32_t i = 0; i < numEntities; ++i) {
        ++entityCount;

        float xPos = static_cast<float>(i % 70) * 3.0f;
        float zPos = static_cast<float>(i / 70) * 3.0f;

        scene.meshSet.add(entityCount, scene.assetManager.meshes[1]);
        scene.materialSet.add(entityCount, scene.assetManager.materials[0]);
        scene.renderableSet.add(entityCount, RenderableTag{});
        scene.velocitySet.add(entityCount, VelocityComponent{glm::vec3(0.0f)});
        scene.speedSet.add(entityCount, SpeedComponent{5.0f});
        glm::vec3 pos = glm::vec3(xPos, 0.5f, zPos);
        scene.transformSet.add(entityCount, TransformComponent{.position = pos});

        if (i % lightFrequency == 0) {
            scene.pointLightSet.add(entityCount, PointLightComponent{
                                                     .colour = glm::vec3(1.0f),
                                                     .intensity = 1.5f,
                                                     .radius = 12.0f});
        }
    }

    ++entityCount;
    scene.meshSet.add(entityCount, createUnitCubePrimitive(scene.assetManager.meshes));
    scene.materialSet.add(entityCount, scene.assetManager.materials[0]);
    scene.renderableSet.add(entityCount, RenderableTag{});
    scene.velocitySet.add(entityCount, VelocityComponent{glm::vec3(0.0f)});
    scene.speedSet.add(entityCount, SpeedComponent{5.0f});
    scene.transformSet.add(entityCount, TransformComponent{.position = glm::vec3(5.0f, 0.5f, 0.0f)});
}

GLuint createSceneUBO() {
    GLuint sceneUBO;
    glGenBuffers(1, &sceneUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, sceneUBO);

    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneUBOData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, sceneUBO);

    return sceneUBO;
}

// TODO: SKYBOX DOESNT HAVE TO BE UPDATED EVERY FRAME ONLY WHEN IT CHANGES, NOT A PRIORITY RIGHT NOW
void updateSceneData(SceneUBOData& sceneData, const CameraComponent& camera, 
                    const std::vector<PackedLightData>& visiblePointLights, const SkyboxData& skyboxData) {
    sceneData.viewMatrix = camera.viewMatrix;
    sceneData.projectionMatrix = camera.projectionMatrix;
    sceneData.cameraPosition = camera.position;
    sceneData.pointLightCount = static_cast<uint32_t>(visiblePointLights.size());
    sceneData.skyboxCubemapHandle = skyboxData.cubemapHandle;
};

void uploadSceneUBO(const GLuint sceneUBO, const SceneUBOData sceneData) {
    glBindBuffer(GL_UNIFORM_BUFFER, sceneUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneUBOData), &sceneData);
}

GLuint createLightSSBO() {
    GLuint lightSSBO;
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);

    // TODO: CHANGE WHERE THIS BELONGS - NEEDS TO BE A MODIFIABLE.
    uint32_t maxLights = 256;
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxLights * sizeof(PackedLightData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

    return lightSSBO;
}

void performLightCulling(const SparseSet<PointLightComponent>& pointLightEntities,
                         const SparseSet<TransformComponent>& transformSet,
                         std::vector<PackedLightData>& visiblePointLights,
                         const glm::vec4* frustumPlanes) {

    visiblePointLights.clear();

    for (uint32_t entity : pointLightEntities.getEntities()) {
        const auto& light = pointLightEntities.getComponent(entity);
        const auto& transform = transformSet.getComponent(entity);

        bool isInside = true;

        for (int i = 0; i < 6; ++i) {
            float distance = glm::dot(glm::vec3(frustumPlanes[i]), transform.position) + frustumPlanes[i].w;

            if (distance < -light.radius) {
                isInside = false;
                break;
            }
        }

        if (isInside) {
            visiblePointLights.emplace_back(glm::vec4(light.colour, light.intensity), glm::vec4(transform.position, light.radius));
        }
    }
}

void uploadLightSSBO(const GLuint lightSSBO, const std::vector<PackedLightData>& visiblePointLights) {
    if (visiblePointLights.size() == 0) return;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, visiblePointLights.size() * sizeof(PackedLightData), visiblePointLights.data());
}

void performFrustumCulling(const std::vector<uint32_t>& renderableEntities, // TODO: KIND OF WEIRD
                           const SparseSet<TransformComponent>& transformSet,
                           const SparseSet<MeshData>& meshSet,
                           std::vector<uint32_t>& visibleEntities,
                           const glm::vec4* frustumPlanes) {

    visibleEntities.clear();
    for (uint32_t entity : renderableEntities) {
        const TransformComponent& transform = transformSet.getComponent(entity);
        const MeshData& mesh = meshSet.getComponent(entity);

        // ARVO METHOD
        // TODO: EXPLAIN ARVO METHOD
        glm::mat3 R = glm::mat3_cast(transform.rotation);

        glm::vec3 localCenter = glm::vec3(
            (mesh.localAABB.minX + mesh.localAABB.maxX) * 0.5f,
            (mesh.localAABB.minY + mesh.localAABB.maxY) * 0.5f,
            (mesh.localAABB.minZ + mesh.localAABB.maxZ) * 0.5f);
        glm::vec3 localExtent = glm::vec3(
            (mesh.localAABB.maxX - mesh.localAABB.minX) * 0.5f,
            (mesh.localAABB.maxY - mesh.localAABB.minY) * 0.5f,
            (mesh.localAABB.maxZ - mesh.localAABB.minZ) * 0.5f);

        glm::vec3 worldCenter = transform.position + (R * (localCenter * transform.scale));

        glm::vec3 worldExtent;
        for (int i = 0; i < 3; ++i) {
            worldExtent[i] =
                glm::abs(R[0][i] * transform.scale.x) * localExtent.x +
                glm::abs(R[1][i] * transform.scale.y) * localExtent.y +
                glm::abs(R[2][i] * transform.scale.z) * localExtent.z;
        }

        bool isInside = true;
        // A frustum is always made up of six planes.
        for (int i = 0; i < 6; ++i) {
            const glm::vec4& plane = frustumPlanes[i];

            float r = worldExtent.x * glm::abs(plane.x) +
                      worldExtent.y * glm::abs(plane.y) +
                      worldExtent.z * glm::abs(plane.z);

            float s = glm::dot(glm::vec3(plane), worldCenter) + plane.w;

            if (s < -r) {
                isInside = false;
                break;
            }
        }

        if (isInside) {
            visibleEntities.push_back(entity);
        }
    }
}