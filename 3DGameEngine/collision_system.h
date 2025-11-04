#pragma once
#include "ecs.h"
#include <glm/glm.hpp>

class CollisionSystem {
public:
    CollisionSystem(ECS& scene);

    void updateVelocity(float deltaTime);
    bool collide(glm::vec3 futurePosition, glm::vec3 obstaclePos,
        CollisionComponent& box1, CollisionComponent& box2);

private:
    ECS& scene;
};

