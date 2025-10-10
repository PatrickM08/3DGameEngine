#pragma once
#include "shader_s.h"
#include <cstdint>
#include <vector>
#include <initializer_list>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>


struct Entity {
    uint32_t id;
};

struct TransformComponent {
    glm::mat4 transform;
};

struct SkyboxTag {
    bool isSkybox; //This should change
};

struct InstancedTag {
    uint32_t numberOfInstances = 0;
};