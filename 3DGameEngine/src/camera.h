#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "sparse_set.h"

struct TransformComponent;
struct CameraComponent;

void updateCameraPosition(SparseSet<TransformComponent>& transformSet, SparseSet<CameraComponent>& cameraSet);
void updateCameraVectors(CameraComponent& camera);
void processMouseScroll(CameraComponent& camera, float yoffset);
void updateProjectionMatrix(CameraComponent& camera, uint32_t windowWidth, uint32_t windowHeight);
void processMouseMovement(CameraComponent& camera, float xoffset, float yoffset, bool constrainPitch = true);
// TODO: It might be better practice to pass in a refereence to the actual attributes rather than the whole struct.
void updateViewMatrix(CameraComponent& camera);
void updateViewProjectionMatrix(CameraComponent& camera);
void updateFrustumPlanes(CameraComponent& camera);