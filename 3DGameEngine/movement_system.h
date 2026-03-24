#pragma once
#include <glm/glm.hpp>
#include "sparse_set.h"

struct TransformComponent;
struct VelocityComponent;
struct SpeedComponent;
struct RotationSpeedComponent;
struct PlayerInputTankTag;
struct PlayerInputWorldTag;
struct PlayerInputNoClipTag;
struct InputMapComponent;
struct PatrolComponent;
struct CameraComponent;

struct InputDirection {
	glm::vec3 direction;
};

void worldSpaceInputSystem(const SparseSet<PlayerInputWorldTag>& inputWorldSet, SparseSet<VelocityComponent>& velocitySet,
                           const SparseSet<SpeedComponent>& speedSet, const SparseSet<InputMapComponent>& inputMapSet, float* keyStateBuffer);

void tankInputSystem(const SparseSet<RotationSpeedComponent>& rotationSpeedSet, const SparseSet<SpeedComponent>& speedSet,
                     const SparseSet<PlayerInputTankTag>& inputTankSet, SparseSet<VelocityComponent>& velocitySet,
                     SparseSet<TransformComponent>& transformSet, float deltaTime, float* keyStateBuffer, 
                     const SparseSet<InputMapComponent>& inputMapSet);

void noClipInputSystem(const SparseSet<PlayerInputNoClipTag>& inputNoClipSet, const SparseSet<SpeedComponent>& speedSet,
                       SparseSet<VelocityComponent>& velocitySet, const SparseSet<InputMapComponent>& inputMapSet,
                       float* keyStateBuffer, const glm::vec3& front, const glm::vec3& right);

    void patrolSystem(SparseSet<PatrolComponent>& patrolSet, const SparseSet<SpeedComponent>& speedSet,
                      SparseSet<VelocityComponent>& velocitySet, float deltaTime);

void movementSystem(const SparseSet<VelocityComponent>& velocitySet, SparseSet<TransformComponent>& transformSet, float deltaTime);