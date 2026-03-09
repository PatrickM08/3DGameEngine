#include "movement_system.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


void worldSpaceInputSystem(const SparseSet<PlayerInputWorldTag>& inputWorldSet, SparseSet<VelocityComponent>& velocitySet,
						   const SparseSet<SpeedComponent>& speedSet, const SparseSet<InputMapComponent>& inputMapSet, float* keyStateBuffer) {
    for (uint32_t i = 0; i < inputWorldSet.entityCount; ++i) {
        uint32_t entity = inputWorldSet.entities[i];
        const InputMapComponent& inputMap = inputMapSet.getComponent(entity);
        glm::vec3 direction(
            static_cast<float>(keyStateBuffer[inputMap.rightIndex] - keyStateBuffer[inputMap.leftIndex]),
            0.0f,
            static_cast<float>(keyStateBuffer[inputMap.backIndex] - keyStateBuffer[inputMap.forwardIndex]));
        float squaredLength = glm::dot(direction, direction);
        float deadzone = 0.0001;
        if (squaredLength > 0.0001)
			// TODO: LOOK INTO X86 INVERSE SQUARE ROOT INSTRUCTIONS
			velocitySet.getComponent(entity).velocity = (direction/glm::sqrt(squaredLength)) * speedSet.getComponent(entity).speed;
        else {
            velocitySet.getComponent(entity).velocity = glm::vec3(0.0f);
		}
	}
}


// TODO: OPTIMISE THIS

void tankInputSystem(const SparseSet<RotationSpeedComponent>& rotationSpeedSet, const SparseSet<SpeedComponent>& speedSet,
                     const SparseSet<PlayerInputTankTag>& inputTankSet, SparseSet<VelocityComponent>& velocitySet,
                     SparseSet<TransformComponent>& transformSet, float deltaTime, float* keyStateBuffer, 
                     const SparseSet<InputMapComponent>& inputMapSet) {
    for (uint32_t i = 0; i < inputTankSet.entityCount; ++i) {
        uint32_t entity = inputTankSet.entities[i];
        const InputMapComponent& inputMap = inputMapSet.getComponent(entity);
		TransformComponent& transform = transformSet.getComponent(entity);
		const RotationSpeedComponent& rotationSpeed = rotationSpeedSet.getComponent(entity);
        const SpeedComponent& speed = speedSet.getComponent(entity);
		VelocityComponent& velocity = velocitySet.getComponent(entity);

		transform.rotation = glm::angleAxis(glm::radians(rotationSpeed.rotationSpeed) *
							 (keyStateBuffer[inputMap.leftIndex] - keyStateBuffer[inputMap.rightIndex]) * deltaTime,
							 glm::vec3(0, 1, 0)) * transform.rotation;

		glm::vec3 forwardVelocity = transform.rotation * glm::vec3(0.0f, 0.0f, keyStateBuffer[inputMap.backIndex] - keyStateBuffer[inputMap.forwardIndex]);
		velocity.velocity = forwardVelocity * speed.speed;
	}
}

void noClipInputSystem(const SparseSet<PlayerInputNoClipTag>& inputNoClipSet, const SparseSet<SpeedComponent>& speedSet, 
					   SparseSet<VelocityComponent>& velocitySet, const SparseSet<InputMapComponent>& inputMapSet, 
					   float* keyStateBuffer, const glm::vec3& front, const glm::vec3& right) {
    for (uint32_t i = 0; i < inputNoClipSet.entityCount; ++i) {
        uint32_t entity = inputNoClipSet.entities[i];
        const InputMapComponent& inputMap = inputMapSet.getComponent(entity);
		VelocityComponent& velocity = velocitySet.getComponent(entity);
        const SpeedComponent& speed = speedSet.getComponent(entity);
        glm::vec3 direction = (front * (keyStateBuffer[inputMap.forwardIndex] - keyStateBuffer[inputMap.backIndex])) + 
							  (right * (keyStateBuffer[inputMap.rightIndex] - keyStateBuffer[inputMap.leftIndex]));
		velocity.velocity = direction * speed.speed;
	}
}


void patrolSystem(SparseSet<PatrolComponent>& patrolSet, const SparseSet<SpeedComponent>& speedSet, 
				  SparseSet<VelocityComponent>& velocitySet, float deltaTime) {
    for (uint32_t i = 0; i < patrolSet.entityCount; ++i) {
        uint32_t entity = patrolSet.entities[i];
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
    for (uint32_t i = 0; i < velocitySet.entityCount; ++i) {
        uint32_t entity = velocitySet.entities[i];
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
