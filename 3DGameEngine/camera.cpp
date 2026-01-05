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
    updateViewProjectionMatrix(camera);
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
    updateViewProjectionMatrix(camera);
}

void updateViewProjectionMatrix(CameraComponent& camera) {
    camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
    updateFrustumPlanes(camera);
}

void updateFrustumPlanes(CameraComponent& camera) {
    const glm::mat4& vp = camera.viewProjectionMatrix;

    // TODO: CHECK PERFORMANCNE OF THIS
    glm::vec4 row0 = glm::vec4(vp[0][0], vp[1][0], vp[2][0], vp[3][0]);
    glm::vec4 row1 = glm::vec4(vp[0][1], vp[1][1], vp[2][1], vp[3][1]);
    glm::vec4 row2 = glm::vec4(vp[0][2], vp[1][2], vp[2][2], vp[3][2]);
    glm::vec4 row3 = glm::vec4(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);

    camera.frustumPlanes[0] = row3 + row0; // Left
    camera.frustumPlanes[1] = row3 - row0; // Right
    camera.frustumPlanes[2] = row3 + row1; // Bottom
    camera.frustumPlanes[3] = row3 - row1; // Top
    camera.frustumPlanes[4] = row3 + row2; // Near
    camera.frustumPlanes[5] = row3 - row2; // Far

    for (int i = 0; i < 6; i++) {
        glm::vec4& plane = camera.frustumPlanes[i];
        float invLength = 1.0f / sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        plane *= invLength;
    }
}