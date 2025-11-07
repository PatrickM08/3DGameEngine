#include "movement_system.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>



WorldSpaceInputSystem::WorldSpaceInputSystem(ECS& scene)
	: scene(scene)
{
}

void WorldSpaceInputSystem::updateVelocity(InputDirection& dir) {
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

void TankInputSystem::updateVelocity(InputDirection& dir, float deltaTime) {
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

		glm::vec3 position = glm::vec3(transform.transform[3]);
		position += velocity.velocity * deltaTime;

		transform.transform = glm::translate(glm::mat4(1.0f), position)
			* glm::mat4_cast(transform.rotation);
		if (scene.cameraSet.hasComponent(entity)) {
			CameraComponent& camera = scene.cameraSet.getComponent(entity);
			updateCameraPosition(camera, position);
			updateViewMatrix(camera);
		}
	}
}