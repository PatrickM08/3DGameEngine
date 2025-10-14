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
    glm::mat4 transform;
    glm::quat rotation;
};

struct InstancedTag {
    uint32_t numberOfInstances = 0;
};

struct VelocityComponent {
    glm::vec3 velocity;
};

struct SpeedComponent {
    float speed;
};

// These need to change
struct PlayerInputWorldTag {
    bool hasPlayerInputWorld;
};

struct PlayerInputTankTag {
    bool hasPlayerInputTank;
};

struct SkyboxTag {
    bool isSkybox;
};