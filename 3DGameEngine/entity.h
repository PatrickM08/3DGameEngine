#pragma once
#include "shader_s.h"
#include <cstdint>
#include <vector>
#include <initializer_list>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GLFW/glfw3.h>


struct Entity {
    uint32_t id;
};

struct TransformComponent {
    glm::mat4 transform = glm::mat4(1.0f);
    glm::quat rotation = glm::quat(1, 0, 0, 0);
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
    glm::vec3 direction = glm::vec3{ 0,0,0 };
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

struct SkyboxTag {};