#pragma once
#include "Window.h" 
#include "render_system.h"
#include "camera.h"
#include "ecs.h"
#include "entity.h"
#include "movement_system.h"
#include "collision_system.h"

class Application {
private:
    Window window;
    GLFWwindow* windowPtr;
    RenderSystem renderSystem;
    bool firstMouse;
    float lastX, lastY;
    float deltaTime, lastFrame;
    ECS scene;
    MovementSystem movementSystem;
    WorldSpaceInputSystem worldSpaceInputSystem;
    TankInputSystem tankInputSystem;
    InputDirection inputDirection;
    PatrolSystem patrolSystem;
    CollisionSystem collisionSystem;

private:
    void handleWindowEvent(const Event& event);
    void handleKeyInput();
    void updateTiming();

public:
    Application();
    int run();
};
