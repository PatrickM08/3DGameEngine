#include "movement_system.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


void worldSpaceInputSystem(const SparseSet<PlayerInputWorldTag>& inputWorldSet, SparseSet<VelocityComponent>& velocitySet,
						   const SparseSet<SpeedComponent>& speedSet, bool* keyStateBuffer) {
    glm::vec3 forwardVector = glm::vec3(0.0f, 0.0f, -1.0f * (float)keyStateBuffer[GLFW_KEY_W - 32]);
	for (uint32_t entity : inputWorldSet.getEntities()) {
        /*
		if (dir.direction != glm::vec3(0.0f, 0.0f, 0.0f)) {
			dir.direction = glm::normalize(dir.direction);
		}
		*/
		velocitySet.getComponent(entity).velocity = forwardVector * speedSet.getComponent(entity).speed;
	}
}


// TODO: THIS SHOULD BE COMPLETELY PARRALEL ON THE VELOCITY COMPONENTS

void tankInputSystem(const SparseSet<RotationSpeedComponent>& rotationSpeedSet, const SparseSet<SpeedComponent>& speedSet, 
					 const SparseSet<PlayerInputTankTag>& inputTankSet, SparseSet<VelocityComponent>& velocitySet, 
					 SparseSet<TransformComponent>& transformSet, const InputDirection& dir, float deltaTime) {
	for (uint32_t entity : inputTankSet.getEntities()) {
		TransformComponent& transform = transformSet.getComponent(entity);
		const RotationSpeedComponent& rotationSpeed = rotationSpeedSet.getComponent(entity);
        const SpeedComponent& speed = speedSet.getComponent(entity);
		VelocityComponent& velocity = velocitySet.getComponent(entity);

		transform.rotation = glm::angleAxis(glm::radians(rotationSpeed.rotationSpeed) * -1 * dir.direction.x * deltaTime,
			glm::vec3(0, 1, 0)) * transform.rotation;

		glm::vec3 forward = transform.rotation * glm::vec3(0.0f, 0.0f, 1.0f);
		velocity.velocity = dir.direction.z * forward * speed.speed;
	}
}

void noClipInputSystem(const SparseSet<PlayerInputNoClipTag>& inputNoClipSet, const SparseSet<SpeedComponent>& speedSet, 
					   SparseSet<VelocityComponent>& velocitySet, InputDirection dir, glm::vec3 front, glm::vec3 right) {
	for (uint32_t entity : inputNoClipSet.getEntities()) {
		auto& velocity = velocitySet.getComponent(entity);
		auto& speed = speedSet.getComponent(entity);
		if (dir.direction != glm::vec3(0.0f, 0.0f, 0.0f)) {
			dir.direction = (front * dir.direction.z * -1.0f) + (right * dir.direction.x); // We're looking down the -z axis
		}
		velocity.velocity = dir.direction * speed.speed;
	}
}


void patrolSystem(SparseSet<PatrolComponent>& patrolSet, const SparseSet<SpeedComponent>& speedSet, 
				  SparseSet<VelocityComponent>& velocitySet, float deltaTime) {
	for (uint32_t entity : patrolSet.getEntities()) {
		auto& patrol = patrolSet.getComponent(entity);
		auto& speed = speedSet.getComponent(entity);
		auto& velocity = velocitySet.getComponent(entity);

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


void movementSystem(const SparseSet<VelocityComponent>& velocitySet, SparseSet<TransformComponent>& transformSet, 
					SparseSet<CameraComponent>& cameraSet, float deltaTime) {
	for (uint32_t entity : velocitySet.getEntities()) {
		auto& transform = transformSet.getComponent(entity);
		auto& velocity = velocitySet.getComponent(entity);

		transform.position += velocity.velocity * deltaTime;

		// TODO: CHANGE THIS
		if (cameraSet.hasComponent(entity)) {
			CameraComponent& camera = cameraSet.getComponent(entity);
			updateCameraPosition(camera, transform.position);
			updateViewMatrix(camera);
		}
	}
}
