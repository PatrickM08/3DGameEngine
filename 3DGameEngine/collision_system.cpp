#include "collision_system.h"
#include <algorithm>

void collisionSystem(SparseSet<CollisionComponent>& collisionSet, SparseSet<TransformComponent>& transformSet,
                     SparseSet<DynamicTag>& dynamicSet, SparseSet<BulletTag>& bulletSet, SparseSet<HealthComponent>& healthSet,
                     CollisionPhysicsManifold& physicsManifold, DeleteBuffer& deleteBuffer) {
    
    const std::vector<uint32_t>& collisionEntities = collisionSet.getEntities();
    uint32_t collisionSetSize = collisionEntities.size();
    const std::vector<uint32_t>& dynamicEntities = dynamicSet.getEntities();
    uint32_t dynamicSetSize = dynamicEntities.size();

    for (uint32_t i = 0; i < dynamicSetSize; i++) {
        uint32_t entity = dynamicEntities[i]; // TODO: THIS WORKS BUT IT MIGHT BE A HACK.
        
        TransformComponent& transform = transformSet.getComponent(entity);
        CollisionComponent& boundingBox = collisionSet.getComponent(entity);

        glm::vec3 position = transform.position;

        for (uint32_t j = 0; j < collisionSetSize; j++) {
            uint32_t obstacleEntity = collisionEntities[j];
            if (entity != obstacleEntity) {
                glm::vec3 obstaclePos = glm::vec3(transformSet.getComponent(obstacleEntity).position);
                CollisionComponent& obstacleBox = collisionSet.getComponent(obstacleEntity);

                float minX1 = position.x + boundingBox.minX;
                float maxX1 = position.x + boundingBox.maxX;
                float minY1 = position.y + boundingBox.minY;
                float maxY1 = position.y + boundingBox.maxY;
                float minZ1 = position.z + boundingBox.minZ;
                float maxZ1 = position.z + boundingBox.maxZ;

                float minX2 = obstaclePos.x + obstacleBox.minX;
                float maxX2 = obstaclePos.x + obstacleBox.maxX;
                float minY2 = obstaclePos.y + obstacleBox.minY;
                float maxY2 = obstaclePos.y + obstacleBox.maxY;
                float minZ2 = obstaclePos.z + obstacleBox.minZ;
                float maxZ2 = obstaclePos.z + obstacleBox.maxZ;

                if (minX1 <= maxX2 && maxX1 >= minX2 &&
                    minY1 <= maxY2 && maxY1 >= minY2 &&
                    minZ1 <= maxZ2 && maxZ1 >= minZ2) {

                    // Using the Minimum Translation Vector (MTV) approach, 
                    // the idea is the shortest overlap would be the direction in which we collided (not always true but it works in practice),
                    // therefore we resolve in this direction (we translate our entity by the depth of the overlap).

                    float overlapX = std::min(maxX1, maxX2) - std::max(minX1, minX2);
                    float overlapY = std::min(maxY1, maxY2) - std::max(minY1, minY2);
                    float overlapZ = std::min(maxZ1, maxZ2) - std::max(minZ1, minZ2);

                    glm::vec3 normal(0.0f);
                    float depth = 0.0f;

                    if (overlapX < overlapY && overlapX < overlapZ) {
                        depth = overlapX;
                        if (position.x < obstaclePos.x) normal = glm::vec3(-1, 0, 0);
                        else normal = glm::vec3(1, 0, 0);
                    } else if (overlapY < overlapZ) {
                        depth = overlapY;
                        if (position.y < obstaclePos.y) normal = glm::vec3(0, -1, 0);
                        else normal = glm::vec3(0, 1, 0);
                    } else {
                        depth = overlapZ;
                        if (position.z < obstaclePos.z) normal = glm::vec3(0, 0, -1);
                        else normal = glm::vec3(0, 0, 1);
                    }

                    // TODO: THIS IS A BAD HACK AND NEEDS TO BE CHANGED, SEPERATE THEM INTO SYSTEMS OR SOMETHING IDK - WE SHOULD KEEP THE MANIFOLD
                    if (bulletSet.hasComponent(entity)) {
                        if (deleteBuffer.size < deleteBuffer.capacity) deleteBuffer.buffer[deleteBuffer.size++] = entity;
                        if (healthSet.hasComponent(obstacleEntity)) {
                            --healthSet.getComponent(obstacleEntity).health;
                        }
                    } else if (physicsManifold.size < physicsManifold.capacity && !bulletSet.hasComponent(obstacleEntity)) {
                        physicsManifold.buffer[physicsManifold.size++] =
                            PhysicsManifoldEntry{.entityID = entity, .depth = depth, .collisionNormal = normal};
                    }

                }
            }
        }
    }
}

void resolveCollisions(CollisionPhysicsManifold& physicsManifold, SparseSet<TransformComponent>& transformSet, 
                       SparseSet<VelocityComponent>& velocitySet) {
    for (int i = 0; i < physicsManifold.size; ++i) {
        uint32_t entity = physicsManifold.buffer[i].entityID;
        float depth = physicsManifold.buffer[i].depth;
        const glm::vec3& normal = physicsManifold.buffer[i].collisionNormal; 
        glm::vec3& velocity = velocitySet.getComponent(entity).velocity;
        glm::vec3& position = transformSet.getComponent(entity).position;

        velocity = velocity - (glm::dot(velocity, normal) * normal);
        position = position + (normal * depth);
    }
}


void healthSystem(SparseSet<HealthComponent>& healthSet, DeleteBuffer& deleteBuffer) {
    for (int i = 0; i < healthSet.dense.size(); ++i) {
        if (healthSet.dense[i].health <= 0 && deleteBuffer.size < deleteBuffer.capacity) {
            deleteBuffer.buffer[deleteBuffer.size++] = healthSet.entities[i];
        }
    }
}

// TODO: FIGURE OUT A SMART WAY TO DO THIS - PROBABLY COMES AFTER THE ARENA SET UP - AND ALSO MAKE 0 INVALID
void deleteSystem(ECS& scene) {
    uint32_t* deleteBuffer = scene.deleteBuffer.buffer;
    uint32_t size = scene.deleteBuffer.size;

    if (size == 0) return;

    std::sort(deleteBuffer, deleteBuffer + size);

    uint32_t last;

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.transformSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.meshSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.materialSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.skyboxSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.instancedSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.inputWorldSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.inputTankSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.inputNoClipSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.velocitySet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.speedSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.rotationSpeedSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.patrolSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.collisionSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.renderableSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.cameraSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.pointLightSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.bulletSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.dynamicSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.healthSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }

    last = UINT32_MAX;
    for (uint32_t i = 0; i < size; ++i) {
        if (deleteBuffer[i] == last) continue;
        scene.inputMapSet.remove(deleteBuffer[i]);
        last = deleteBuffer[i];
    }
}