#include "movement_system.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>



WorldSpaceInputSystem::WorldSpaceInputSystem(ECS& scene)
	: scene(scene)
{
}

void WorldSpaceInputSystem::updateVelocity(InputDirection dir) {
	for (uint32_t entity : scene.inputWorldSet.getEntities()) {
		if (dir.direction != glm::vec3(0.0f, 0.0f, 0.0f)) {
			dir.direction = glm::normalize(dir.direction);
		}
		scene.velocitySet.getComponent(entity).velocity = dir.direction * scene.speedSet.getComponent(entity).speed;
	}
}



TankInputSystem::TankInputSystem(ECS& scene)
	: scene(scene)
{
}

void TankInputSystem::updateVelocity(InputDirection dir, float deltaTime) {
	for (uint32_t entity : scene.inputTankSet.getEntities()) {
		auto& transform = scene.transformSet.getComponent(entity);
		auto& rotationSpeed = scene.rotationSpeedSet.getComponent(entity);
		auto& velocity = scene.velocitySet.getComponent(entity);
		auto& speed = scene.speedSet.getComponent(entity);

		transform.rotation = glm::angleAxis(glm::radians(rotationSpeed.rotationSpeed) * -1 * dir.direction.x * deltaTime,
			glm::vec3(0, 1, 0)) * transform.rotation;

		glm::vec3 forward = transform.rotation * glm::vec3(0.0f, 0.0f, 1.0f);
		velocity.velocity = dir.direction.z * forward * speed.speed;
	}
}


NoClipInputSystem::NoClipInputSystem(ECS& scene)
	: scene(scene)
{
}

void NoClipInputSystem::updateVelocity(InputDirection dir, glm::vec3 front, glm::vec3 right) {
	for (uint32_t entity : scene.inputNoClipSet.getEntities()) {
		auto& velocity = scene.velocitySet.getComponent(entity);
		auto& speed = scene.speedSet.getComponent(entity);
		if (dir.direction != glm::vec3(0.0f, 0.0f, 0.0f)) {
			dir.direction = (front * dir.direction.z * -1.0f) + (right * dir.direction.x); // We're looking down the -z axis
		}
		velocity.velocity = dir.direction * speed.speed;
	}
}


PatrolSystem::PatrolSystem(ECS& scene)
	: scene(scene)
{
}

void PatrolSystem::updateVelocity(float deltaTime) {
	for (uint32_t entity : scene.patrolSet.getEntities()) {
		auto& patrol = scene.patrolSet.getComponent(entity);
		auto& speed = scene.speedSet.getComponent(entity);
		auto& velocity = scene.velocitySet.getComponent(entity);

		if (patrol.magnitude != 0) {
			if (patrol.currentPatrolDistance < patrol.magnitude) {
				velocity.velocity = patrol.direction * speed.speed;
				patrol.currentPatrolDistance += speed.speed * deltaTime;
			}
			else {
				patrol.direction = patrol.direction * -1.0f;
				patrol.currentPatrolDistance = 0;
			}
		}
	}
}



MovementSystem::MovementSystem(ECS& scene)
	: scene(scene)
{
}

void MovementSystem::updateTransforms(float deltaTime) {
	for (uint32_t entity : scene.velocitySet.getEntities()) {
		auto& transform = scene.transformSet.getComponent(entity);
		auto& velocity = scene.velocitySet.getComponent(entity);

		transform.position += velocity.velocity * deltaTime;

		// TODO: CHANGE THIS
		if (scene.cameraSet.hasComponent(entity)) {
			CameraComponent& camera = scene.cameraSet.getComponent(entity);
			updateCameraPosition(camera, transform.position);
			updateViewMatrix(camera);
		}
	}
}
