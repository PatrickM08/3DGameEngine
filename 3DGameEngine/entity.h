#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Entity {
    uint32_t id;
};

// It would be better if these were seperate - at least scale, because its just taking up room in cache thats hardly used.
struct TransformComponent {
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
};

struct InstancedComponent {
    uint32_t numberOfInstances = 0;
};

struct VelocityComponent {
    glm::vec3 velocity;
};

struct SpeedComponent {
    float speed;
};

struct RotationSpeedComponent {
    float rotationSpeed;
};

struct PatrolComponent {
    glm::vec3 direction = glm::vec3{0, 0, 0};
    float magnitude = 0.0f;
    float currentPatrolDistance = 0.0f;
};

struct CollisionComponent {
    float minX = 0, maxX = 0;
    float minY = 0, maxY = 0;
    float minZ = 0, maxZ = 0;
};

struct RenderableTag {};

struct PlayerInputWorldTag {};

struct PlayerInputTankTag {};

struct PlayerInputNoClipTag {};

struct SkyboxTag {};
