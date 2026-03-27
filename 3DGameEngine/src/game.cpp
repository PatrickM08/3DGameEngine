#include "ecs.h"
#include "movement_system.h"
#include "collision_system.h"

void createBullet(ECS& scene, glm::vec3 position, glm::quat rotation) {
    uint32_t id = createEntity(scene);
    scene.meshSet.add(id, scene.meshBuffer.buffer[scene.cubePrimitiveIndex]);
    scene.materialSet.add(id, scene.materialBuffer.buffer[0]);
    scene.renderableSet.add(id, RenderableTag{});
    glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    float bulletOffset = 1.0f;
    position += bulletOffset * front;
    scene.transformSet.add(id, TransformComponent{.position = position, .scale = glm::vec3(0.2f)});
    scene.collisionSet.add(id, CollisionComponent{.minX = -0.1f, .maxX = 0.1f, .minY = -0.1f, .maxY = 0.1f, .minZ = -0.1f, .maxZ = 0.1f});
    scene.velocitySet.add(id, VelocityComponent{glm::vec3(10.0f * front)});
    scene.bulletSet.add(id, BulletTag{});
    scene.dynamicSet.add(id, DynamicTag{});
}

void bulletSystem(ECS& scene) {
    SparseSet<InputMapComponent>& inputMapSet = scene.inputMapSet;
    float* keyStateBuffer = scene.keyStateBuffer;
    float* lastKeyStateBuffer = scene.lastKeyStateBuffer;
    SparseSet<TransformComponent>& transformSet = scene.transformSet;
    for (uint32_t i = 0; i < inputMapSet.entityCount; ++i) {
        uint32_t entity = inputMapSet.entities[i];
        const TransformComponent& transform = transformSet.getComponent(entity);
        const InputMapComponent& inputMap = inputMapSet.getComponent(entity);
        if (keyStateBuffer[inputMap.shootIndex] && !lastKeyStateBuffer[inputMap.shootIndex]) {
            createBullet(scene, transform.position, transform.rotation);
        }
    }
}

extern "C" __declspec(dllexport) void game_update(ECS& scene, CameraComponent& camera) {
    worldSpaceInputSystem(scene.inputWorldSet, scene.velocitySet, scene.speedSet, scene.inputMapSet, scene.keyStateBuffer);
    tankInputSystem(scene.rotationSpeedSet, scene.speedSet, scene.inputTankSet,
                    scene.velocitySet, scene.transformSet, scene.deltaTime, scene.keyStateBuffer, scene.inputMapSet);
    noClipInputSystem(scene.inputNoClipSet, scene.speedSet, scene.velocitySet, scene.inputMapSet, scene.keyStateBuffer, camera.front, camera.right);
    patrolSystem(scene.patrolSet, scene.speedSet, scene.velocitySet, scene.deltaTime);
    movementSystem(scene.velocitySet, scene.transformSet, scene.deltaTime);
    bulletSystem(scene);
    collisionSystem(scene.collisionSet, scene.transformSet, scene.dynamicSet, scene.bulletSet, scene.healthSet,
                    scene.physicsManifold, scene.deleteBuffer);
    resolveCollisions(scene.physicsManifold, scene.transformSet, scene.velocitySet);
    healthSystem(scene.healthSet, scene.deleteBuffer);
    deleteSystem(scene);
}