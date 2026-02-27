#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Entity {
    uint32_t id;
};

// TODO: SEEMS A BIT WASTEFUL TO HAVE THIS AND PACKEDLIGHTDATA, CHECK NECESSITY
struct PointLightComponent {
    glm::vec3 colour = glm::vec3(1, 1, 1);
    float intensity = 1.0f;
    float radius = 10.0f;
};

// TODO: THIS MIGHT NOT BELONG HERE, COULD PACK COLOUR FURTHER USING BIT SHIFTING
struct PackedLightData {
    glm::vec4 colourAndIntensity;
    glm::vec4 positionAndRadius;
};

// TODO: It would be better if these were seperate - at least scale, because its just taking up room in cache thats hardly used.
// MAYBE NOT ACTUALLY
struct TransformComponent {
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 position = glm::vec3(0, 0, 0);
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

struct HealthComponent {
    int32_t health; // TODO: THIS COULD BE 8 BYTE - DEPENDING ON GAME - ADD OPTIONS, WOULD BE ABLE TO FIT MORE INTO CACHE. ALSO MIGHT WRAPAROUND.
};

struct InputMapComponent {
    uint16_t forwardIndex;
    uint16_t backIndex;
    uint16_t leftIndex;
    uint16_t rightIndex;
    uint16_t shootIndex;
};

struct RenderableTag {};

struct PlayerInputWorldTag {};

struct PlayerInputTankTag {};

struct PlayerInputNoClipTag {};

struct SkyboxTag {};

struct BulletTag {};

struct DynamicTag {};
