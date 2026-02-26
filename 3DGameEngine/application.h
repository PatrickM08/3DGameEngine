#pragma once
#include "ecs.h"

struct Application {
    float deltaTime, lastFrame;
    ECS scene;
};

void handleKeyInput(ECS& scene);
void updateTiming(Application& app);
int run(Application& app);
