#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "sparse_set.h"

struct ECS;
struct CollisionComponent;
struct TransformComponent;
struct DynamicTag;
struct BulletTag;
struct HealthComponent;
struct VelocityComponent;

struct PhysicsManifoldEntry {
    uint32_t entityID;
    float depth;
    glm::vec3 collisionNormal;
};

// TODO: The capacity and size of these can probably be 16 bit - but we can leave it for now - MAKE THESE ONE BUFFER STRUCT?
struct CollisionPhysicsManifold {
    static constexpr uint32_t capacity = 64;
    uint32_t size = 0;
    PhysicsManifoldEntry buffer[capacity];
};

struct DeleteBuffer {
    static constexpr uint32_t capacity = 64;
    uint32_t size = 0;
    uint32_t buffer[capacity];
};

void collisionSystem(SparseSet<CollisionComponent>& collisionSet, SparseSet<TransformComponent>& transformSet,
                     SparseSet<DynamicTag>& dynamicSet, SparseSet<BulletTag>& bulletSet, SparseSet<HealthComponent>& healthSet,
                     CollisionPhysicsManifold& physicsManifold, DeleteBuffer& deleteBuffer);

void resolveCollisions(CollisionPhysicsManifold& physicsManifold, SparseSet<TransformComponent>& transformSet,
                       SparseSet<VelocityComponent>& velocitySet);

void healthSystem(SparseSet<HealthComponent>& healthSet, DeleteBuffer& deleteBuffer);

void deleteSystem(ECS& scene);
