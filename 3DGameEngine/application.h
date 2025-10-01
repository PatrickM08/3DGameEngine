#pragma once
#include "Window.h" 
#include "render_system.h"
#include "camera.h"
#include "asset_manager.h"
#include "ecs.h"
#include "entity.h"

class Application {
private:
    Window window;
    GLFWwindow* windowPtr;
    RenderSystem renderSystem;
    Camera camera;
    bool firstMouse;
    float lastX, lastY;
    float deltaTime, lastFrame;
    AssetManager assetManager;
    ECS scene;

private:
    void handleEvent(const Event& event);
    void handleKeyInput();
    void updateTiming();

public:
    Application();
    int run();
};
