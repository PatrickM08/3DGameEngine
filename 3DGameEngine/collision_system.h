#pragma once
#include "ecs.h"
#include <glm/glm.hpp>

class CollisionSystem {
public:
	CollisionSystem(ECS& scene)
		: scene(scene)
	{
	}

	void updateVelocity(float deltaTime) {
		for (int i = 0; i < scene.entitiesInScene.size(); i++) {
			Entity& entity = scene.entitiesInScene[i];
			TransformComponent& transform = scene.transformsInScene[entity.id];
			CollisionComponent& boundingBox = scene.collisionEntities[entity.id];
			VelocityComponent& velocity = scene.velocitiesOfEntities[entity.id];
			for (int j = 0; j < scene.entitiesInScene.size(); j++) {
				if (i != j) {
					glm::vec3 futurePosition = glm::vec3(transform.transform[3]) + velocity.velocity * deltaTime;
					glm::vec3 obstaclePos = glm::vec3(scene.transformsInScene[j].transform[3]);
					if (collide(futurePosition, obstaclePos, boundingBox, scene.collisionEntities[j])) {
						velocity.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
			}
		}
	}

	bool collide(glm::vec3 futurePosition, glm::vec3 obstaclePos, CollisionComponent& box1, CollisionComponent& box2) {
		if (box1.minX == 0 && box1.maxX == 0 && box1.minY == 0 &&
			box1.maxY == 0 && box1.minZ == 0 && box1.maxZ == 0) return false;
		if (box2.minX == 0 && box2.maxX == 0 && box2.minY == 0 &&
			box2.maxY == 0 && box2.minZ == 0 && box2.maxZ == 0) return false;

		float minX1 = futurePosition.x + box1.minX;
		float maxX1 = futurePosition.x + box1.maxX;
		float minY1 = futurePosition.y + box1.minY;
		float maxY1 = futurePosition.y + box1.maxY;
		float minZ1 = futurePosition.z + box1.minZ;
		float maxZ1 = futurePosition.z + box1.maxZ;

		float minX2 = obstaclePos.x + box2.minX;
		float maxX2 = obstaclePos.x + box2.maxX;
		float minY2 = obstaclePos.y + box2.minY;
		float maxY2 = obstaclePos.y + box2.maxY;
		float minZ2 = obstaclePos.z + box2.minZ;
		float maxZ2 = obstaclePos.z + box2.maxZ;

		return (minX1 <= maxX2 &&
			maxX1 >= minX2 &&
			minY1 <= maxY2 &&
			maxY1 >= minY2 &&
			minZ1 <= maxZ2 &&
			maxZ1 >= minZ2);
	}

private:
	ECS& scene;
};