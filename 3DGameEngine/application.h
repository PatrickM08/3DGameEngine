#pragma once
#include "Window.h" 
#include "render_system.h"
#include "camera.h"

class Application {
private:
    Window window;
    GLFWwindow* windowPtr;
    RenderSystem renderSystem;
    Camera camera;
    bool firstMouse;
    float lastX, lastY;
    float deltaTime, lastFrame;
    ECS scene;

private:
    void handleEvent(const Event& event);
    void handleKeyInput();
    void updateTiming();

public:
    Application();
    int run();
};
