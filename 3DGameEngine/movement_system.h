#pragma once
#include "ecs.h"
#include "entity.h"

struct InputDirection {
	glm::vec3 direction;
};

void worldSpaceInputSystem(const SparseSet<PlayerInputWorldTag>& inputWorldSet, SparseSet<VelocityComponent>& velocitySet,
                           const SparseSet<SpeedComponent>& speedSet, bool* keyStateBuffer);

void tankInputSystem(const SparseSet<RotationSpeedComponent>& rotationSpeedSet, const SparseSet<SpeedComponent>& speedSet,
                     const SparseSet<PlayerInputTankTag>& inputTankSet, SparseSet<VelocityComponent>& velocitySet,
                     SparseSet<TransformComponent>& transformSet, const InputDirection& dir, float deltaTime);

void noClipInputSystem(const SparseSet<PlayerInputNoClipTag>& inputNoClipSet, const SparseSet<SpeedComponent>& speedSet,
                       SparseSet<VelocityComponent>& velocitySet, InputDirection dir, glm::vec3 front, glm::vec3 right);

void patrolSystem(SparseSet<PatrolComponent>& patrolSet, const SparseSet<SpeedComponent>& speedSet,
                  SparseSet<VelocityComponent>& velocitySet, float deltaTime);

void movementSystem(const SparseSet<VelocityComponent>& velocitySet, SparseSet<TransformComponent>& transformSet,
                    SparseSet<CameraComponent>& cameraSet, float deltaTime);