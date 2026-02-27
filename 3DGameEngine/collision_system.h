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
