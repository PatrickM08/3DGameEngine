#pragma once
#include "ecs.h"

struct Application {
    float deltaTime, lastFrame;
    ECS* scene = new ECS();
};

void updateTiming(Application& app);
int run(Application& app);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);