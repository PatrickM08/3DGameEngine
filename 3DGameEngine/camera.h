#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraType {
    FIXED,
    MOUSETURN
};

struct CameraComponent {
    glm::vec3 positionOffset;
    glm::vec3 position;
    float nearPlane = 0.1;
    float farPlane = 100;
    bool projectionDirty; // Dirty flag to tell system when to recalculate projection matrix
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float mouseSensitivity = 0.1f;
    float zoom = 45.0f;
    CameraType cameraType = CameraType::FIXED;
};

void updateCameraPosition(CameraComponent& camera, glm::vec3 pos);
void updateCameraVectors(CameraComponent& camera);
void processMouseScroll(CameraComponent& camera, float yoffset);
void updateProjectionMatrix(CameraComponent& camera, uint32_t windowWidth, uint32_t windowHeight);
void processMouseMovement(CameraComponent& camera, float xoffset, float yoffset, bool constrainPitch = true);
void updateViewMatrix(CameraComponent& camera);
