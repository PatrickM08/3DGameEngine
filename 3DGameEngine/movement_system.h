#pragma once
#include "ecs.h"
#include "entity.h"

struct InputDirection {
	glm::vec3 direction;
};

class WorldSpaceInputSystem {
public:
	WorldSpaceInputSystem(ECS& scene);
	void updateVelocity(InputDirection& dir);

private:
	ECS& scene;
};

class TankInputSystem {
public:
	TankInputSystem(ECS& scene);
	void updateVelocity(InputDirection& dir, float deltaTime);

private:
	ECS& scene;
};

class PatrolSystem {
public:
	PatrolSystem(ECS& scene);
	void updateVelocity(float deltaTime);

private:
	ECS& scene;
};

class MovementSystem {
public:
	MovementSystem(ECS& scene);
	void updateTransforms(float deltaTime);

private:
	ECS& scene;
};
