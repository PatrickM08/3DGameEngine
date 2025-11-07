#include "camera.h"
#include <iostream>

void updateCameraPosition(CameraComponent& camera, glm::vec3 pos) {
    camera.position = pos + camera.positionOffset;
}

void updateCameraVectors(CameraComponent& camera) {
    glm::vec3 front;
    front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    front.y = sin(glm::radians(camera.pitch));
    front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    camera.front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}


void processMouseScroll(CameraComponent& camera, float yoffset)
{
    camera.zoom -= (float)yoffset;
    if (camera.zoom < 1.0f)
        camera.zoom = 1.0f;
    if (camera.zoom > 45.0f)
        camera.zoom = 45.0f;
}


void updateProjectionMatrix(CameraComponent& camera, uint32_t windowWidth, uint32_t windowHeight) {
    camera.projectionMatrix = glm::perspective(glm::radians(camera.zoom), (float)windowWidth / (float)windowHeight,
        camera.nearPlane, camera.farPlane);
}

// Maybe constrain pitch should be a camera member
void processMouseMovement(CameraComponent& camera, float xoffset, float yoffset, bool constrainPitch)
{
    if (camera.cameraType != CameraType::FIXED) {
        xoffset *= camera.mouseSensitivity;
        yoffset *= camera.mouseSensitivity;

        camera.yaw += xoffset;
        camera.pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (camera.pitch > 89.0f)
                camera.pitch = 89.0f;
            if (camera.pitch < -89.0f)
                camera.pitch = -89.0f;
        }
        updateCameraVectors(camera);
    }
}

void updateViewMatrix(CameraComponent& camera) {
    camera.viewMatrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
}