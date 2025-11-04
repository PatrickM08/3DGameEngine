#include "collision_system.h"

CollisionSystem::CollisionSystem(ECS& scene)
    : scene(scene)
{
}

void CollisionSystem::updateVelocity(float deltaTime) {
    auto& collisionEntities = scene.collisionSet.getEntities();
    uint32_t collisionSetSize = collisionEntities.size();

    for (uint32_t i = 0; i < collisionSetSize; i++) {
        uint32_t entity = collisionEntities[i];

        auto& transform = scene.transformSet.getComponent(entity);
        auto& boundingBox = scene.collisionSet.getComponent(entity);
        auto& velocity = scene.velocitySet.getComponent(entity);

        glm::vec3 currentPos = glm::vec3(transform.transform[3]);
        glm::vec3 adjustedVelocity = velocity.velocity;

        for (uint32_t j = 0; j < collisionSetSize; j++) {
            uint32_t obstacleEntity = collisionEntities[j];
            if (entity != obstacleEntity) {
                glm::vec3 obstaclePos =
                    glm::vec3(scene.transformSet.getComponent(obstacleEntity).transform[3]);
                CollisionComponent& obstacleBox = scene.collisionSet.getComponent(obstacleEntity);

                glm::vec3 testPos = currentPos + glm::vec3(adjustedVelocity.x, 0, 0) * deltaTime;
                if (collide(testPos, obstaclePos, boundingBox, obstacleBox)) {
                    adjustedVelocity.x = 0.0f;
                }

                testPos = currentPos + glm::vec3(0, adjustedVelocity.y, 0) * deltaTime;
                if (collide(testPos, obstaclePos, boundingBox, obstacleBox)) {
                    adjustedVelocity.y = 0.0f;
                }

                testPos = currentPos + glm::vec3(0, 0, adjustedVelocity.z) * deltaTime;
                if (collide(testPos, obstaclePos, boundingBox, obstacleBox)) {
                    adjustedVelocity.z = 0.0f;
                }
            }
        }

        velocity.velocity = adjustedVelocity;
    }
}

bool CollisionSystem::collide(glm::vec3 futurePosition, glm::vec3 obstaclePos,
    CollisionComponent& box1, CollisionComponent& box2) {
    if ((box1.minX == 0 && box1.maxX == 0 && box1.minY == 0 && box1.maxY == 0 &&
        box1.minZ == 0 && box1.maxZ == 0) ||
        (box2.minX == 0 && box2.maxX == 0 && box2.minY == 0 && box2.maxY == 0 &&
            box2.minZ == 0 && box2.maxZ == 0))
        return false;

    float minX1 = futurePosition.x + box1.minX;
    float maxX1 = futurePosition.x + box1.maxX;
    float minY1 = futurePosition.y + box1.minY;
    float maxY1 = futurePosition.y + box1.maxY;
    float minZ1 = futurePosition.z + box1.minZ;
    float maxZ1 = futurePosition.z + box1.maxZ;

    float minX2 = obstaclePos.x + box2.minX;
    float maxX2 = obstaclePos.x + box2.maxX;
    float minY2 = obstaclePos.y + box2.minY;
    float maxY2 = obstaclePos.y + box2.maxY;
    float minZ2 = obstaclePos.z + box2.minZ;
    float maxZ2 = obstaclePos.z + box2.maxZ;

    return (minX1 <= maxX2 && maxX1 >= minX2 &&
        minY1 <= maxY2 && maxY1 >= minY2 &&
        minZ1 <= maxZ2 && maxZ1 >= minZ2);
}
