#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "sparse_set.h"
#include "entity.h"
#include "asset_manager.h"
#include "camera.h"
#include "collision_system.h"
#include "window.h"
#include "events.h"
#include "text.h"
#include "render_system.h"

struct ECS {
    float deltaTime, lastFrame;

    Arena arena;
    SparseSet<TransformComponent> transformSet;
    SparseSet<MeshData> meshSet;
    SparseSet<MaterialData> materialSet;
    SparseSet<PlayerInputWorldTag> inputWorldSet;
    SparseSet<PlayerInputTankTag> inputTankSet;
    SparseSet<PlayerInputNoClipTag> inputNoClipSet;
    SparseSet<VelocityComponent> velocitySet;
    SparseSet<SpeedComponent> speedSet;
    SparseSet<RotationSpeedComponent> rotationSpeedSet;
    SparseSet<PatrolComponent> patrolSet;
    SparseSet<CollisionComponent> collisionSet;
    SparseSet<RenderableTag> renderableSet;
    SparseSet<CameraComponent> cameraSet;
    SparseSet<PointLightComponent> pointLightSet;
    SparseSet<BulletTag> bulletSet;
    SparseSet<DynamicTag> dynamicSet;
    SparseSet<HealthComponent> healthSet;
    SparseSet<InputMapComponent> inputMapSet;
    SparseSet<NameComponent> nameSet;

    WindowData window;
    
    MeshBuffer meshBuffer;
    uint32_t cubePrimitiveIndex;
    MaterialBuffer materialBuffer;
    MaterialSSBODataBuffer materialSSBODataBuffer;
    uint32_t materialSSBO;

    uint32_t entityCount = 0;
    uint32_t freeStack[64]; // Right now this is the same capacity as the delete buffer which makes sense,
                            // a change in the delete buffer should be a change in this so probably should be a defined capacity.
    uint8_t freeStackSize = 0;

    Framebuffer framebuffer;
    uint32_t quadVAO;
    
    VisibleEntityBuffer visibleEntityBuffer;
    VisiblePointLightBuffer visiblePointLightBuffer;
    uint32_t lightSSBO;
    SkyboxData skyboxData;
    uint32_t sceneUBO;
    SceneUBOData sceneData;

    CollisionPhysicsManifold physicsManifold;
    DeleteBuffer deleteBuffer;
    // TODO: so the idea is that allowing 0 as null will make it easier to have a null mapping, but I need to check performance compared to tags.
    float keyStateBuffer[318];
    float lastKeyStateBuffer[318];
    EventQueue eventQueue;
    MouseData mouseData;

    TextBuffer textBuffer;
    TextRenderData textRenderData;

    uint32_t currentCamera;

    bool debugMode = true;
    int lastPressedGLFWKey = -1;
    int awaitingBind = -1;
    int selectedEntity = -1;
    glm::vec3 pendingDirection = glm::vec3(0.0f, 0.0f, 1.0f);
    float pendingMagnitude = 5.0f;
    char fileNameBuffer[64] = "scene.bin";
};

void initState(ECS& scene);

void initScene(ECS& scene);

inline uint32_t createEntity(ECS& scene) {
    if (scene.freeStackSize > 0) {
        return scene.freeStack[--scene.freeStackSize];
    }
    return ++scene.entityCount;
}

inline uint32_t createCube(ECS& scene, glm::vec3 pos, glm::vec3 scale, uint32_t materialID) {
    uint32_t id = createEntity(scene);

    scene.meshSet.add(id, scene.meshBuffer.buffer[scene.cubePrimitiveIndex]);
    scene.materialSet.add(id, scene.materialBuffer.buffer[materialID]);
    scene.renderableSet.add(id, RenderableTag{});
    scene.velocitySet.add(id, VelocityComponent{glm::vec3(0.0f)});
    scene.transformSet.add(id, TransformComponent{.position = pos, .scale = scale});

    glm::vec3 half = scale * 0.5f;
    scene.collisionSet.add(id, CollisionComponent{
                                   .minX = -half.x, .maxX = half.x, .minY = -half.y, .maxY = half.y, .minZ = -half.z, .maxZ = half.z});

    return id;
}

void updateTiming(ECS& scene);