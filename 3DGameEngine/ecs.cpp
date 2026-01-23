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
}

void init(ECS& scene) {
    uint32_t entityCount = 0;

    ++entityCount;
    scene.meshSet.add(entityCount, scene.assetManager.meshes[1]);
    scene.velocitySet.add(entityCount, VelocityComponent{glm::vec3(0.0f)});
    scene.speedSet.add(entityCount, SpeedComponent{5.0f});
    scene.transformSet.add(entityCount, TransformComponent{glm::vec3(0.0f, 0.0f, 0.0f)});
    scene.inputNoClipSet.add(entityCount, PlayerInputNoClipTag{});
    scene.pointLightSet.add(entityCount, PointLightComponent{.colour = glm::vec3(1.0f), .intensity = 1.0f, .radius = 10.0f});

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
    // TODO: THESE ARE NECESSARY FOR COLLISION OBJECTS - MAYBE SHOULD DISTINGUISH
    scene.transformSet.add(entityCount, TransformComponent{glm::vec3(-20, 0, 20)});

    ++entityCount;
    scene.meshSet.add(entityCount, scene.assetManager.meshes[1]);
    scene.materialSet.add(entityCount, scene.assetManager.materials[0]);
    scene.renderableSet.add(entityCount, RenderableTag{});
    scene.velocitySet.add(entityCount, VelocityComponent{glm::vec3(0.0f)});
    scene.speedSet.add(entityCount, SpeedComponent{5.0f});
    scene.transformSet.add(entityCount, TransformComponent{glm::vec3(0.0f, 0.5f, 0.0f)});

    ++entityCount;
    // TODO: THIS NEEDS TO BE CHANGED HOW THE PRIMITIVES ARE USED
    scene.meshSet.add(entityCount, createUnitCubePrimitive(scene.assetManager.meshes));
    scene.materialSet.add(entityCount, scene.assetManager.materials[0]);
    scene.renderableSet.add(entityCount, RenderableTag{});
    scene.velocitySet.add(entityCount, VelocityComponent{glm::vec3(0.0f)});
    scene.speedSet.add(entityCount, SpeedComponent{5.0f});
    scene.transformSet.add(entityCount, TransformComponent{glm::vec3(5.0f, 0.5f, 0.0f)});
}