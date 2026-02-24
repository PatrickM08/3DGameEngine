#pragma once
#include "ecs.h"
#include <glm/glm.hpp>

void collisionSystem(SparseSet<CollisionComponent>& collisionSet, SparseSet<TransformComponent>& transformSet,
                     SparseSet<DynamicTag>& dynamicSet, SparseSet<BulletTag>& bulletSet, SparseSet<HealthComponent>& healthSet,
                     CollisionPhysicsManifold& physicsManifold, DeleteBuffer& deleteBuffer);

void resolveCollisions(CollisionPhysicsManifold& physicsManifold, SparseSet<TransformComponent>& transformSet,
                       SparseSet<VelocityComponent>& velocitySet);

void healthSystem(SparseSet<HealthComponent>& healthSet, DeleteBuffer& deleteBuffer);

void deleteSystem(ECS& scene);

inline void createBullet(ECS& scene, glm::vec3 position, glm::quat rotation) {
    ++scene.entityCount;
    scene.meshSet.add(scene.entityCount, createUnitCubePrimitive(scene.assetManager.meshes));
    scene.materialSet.add(scene.entityCount, scene.assetManager.materials[0]);
    scene.renderableSet.add(scene.entityCount, RenderableTag{});
    glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    float bulletOffset = 1.0f;
    position += bulletOffset * front;
    scene.transformSet.add(scene.entityCount, TransformComponent{.position = position, .scale = glm::vec3(0.2f)});
    scene.collisionSet.add(scene.entityCount, CollisionComponent{.minX = -0.1f, .maxX = 0.1f, .minY = -0.1f, .maxY = 0.1f, .minZ = -0.1f, .maxZ = 0.1f});
    scene.velocitySet.add(scene.entityCount, VelocityComponent{glm::vec3(5.0f * front)});
    scene.bulletSet.add(scene.entityCount, BulletTag{});
    scene.dynamicSet.add(scene.entityCount, DynamicTag{});
}