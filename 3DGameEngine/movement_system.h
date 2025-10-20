#pragma once
#include "ecs.h"
#include "entity.h"


struct InputDirection {
	glm::vec3 direction;
};

class WorldSpaceInputSystem {
public:
	WorldSpaceInputSystem(ECS& scene) 
		: scene(scene)
	{
	}

	void updateVelocity(InputDirection& dir){
		for (Entity& entity : scene.entitiesInScene) {
			if (scene.playerInputWorldEntities[entity.id].hasPlayerInputWorld) {
				if (dir.direction != glm::vec3(0.0f, 0.0f, 0.0f)) {
					dir.direction = glm::normalize(dir.direction);
				}
				scene.velocitiesOfEntities[entity.id].velocity = dir.direction * scene.speedsOfEntities[entity.id].speed;
			}
		}
	}

private:
	ECS& scene;
};

class TankInputSystem {
public:
	TankInputSystem(ECS& scene)
		: scene(scene)
	{
	}

	void updateVelocity(InputDirection& dir, float deltaTime) {
		for (Entity& entity : scene.entitiesInScene) {
			if (scene.playerInputTankEntities[entity.id].hasPlayerInputTank) {

				scene.transformsInScene[entity.id].rotation = glm::angleAxis(glm::radians(scene.rotationSpeedsOfEntities[entity.id]
					.rotationSpeed) * -1 * dir.direction.x * deltaTime, glm::vec3(0, 1, 0)) * 
					scene.transformsInScene[entity.id].rotation;
				glm::vec3 forward = scene.transformsInScene[entity.id].rotation * glm::vec3(0.0f, 0.0f, 1.0f);
				scene.velocitiesOfEntities[entity.id].velocity = dir.direction.z * forward * scene.speedsOfEntities[entity.id].speed;
			}
		}
	}

private:
	ECS& scene;
};

class PatrolSystem {
public:
	PatrolSystem(ECS& scene)
		: scene(scene)
	{
	}

	void updateVelocity(float deltaTime) {
		for (Entity& entity : scene.entitiesInScene) {
			PatrolComponent& patrol = scene.patrolEntities[entity.id];
			SpeedComponent& speed = scene.speedsOfEntities[entity.id];
			VelocityComponent& velocity = scene.velocitiesOfEntities[entity.id];
			if (patrol.magnitude != 0) {
				if (patrol.currentPatrolDistance < patrol.magnitude) {
					velocity.velocity = patrol.direction * speed.speed;
					patrol.currentPatrolDistance += speed.speed * deltaTime;
				}
				else {
					patrol.direction = patrol.direction * -1.0f;
					patrol.currentPatrolDistance = 0;
					std::cout << "done" << std::endl;
				}
			}
		}
	}

private:
	ECS& scene;
};

class MovementSystem {
public:
	MovementSystem(ECS& scene)
		: scene(scene)
	{
	}

	void updateTransforms(float deltaTime) {
		for (Entity& entity : scene.entitiesInScene) {
			auto& transform = scene.transformsInScene[entity.id];
			auto& velocity = scene.velocitiesOfEntities[entity.id];

			glm::vec3 position = glm::vec3(transform.transform[3]);
			position += velocity.velocity * deltaTime;

			transform.transform = glm::translate(glm::mat4(1.0f), position)
				* glm::mat4_cast(transform.rotation);
		}
	}

private:
	ECS& scene;
};
