#include "Application.h"
#include "render_system.h"
#include "ecs.h"
#include "movement_system.h"
#include "camera.h"
#include <iostream>
#include "entity.h"
#include "sparse_set.h"


Application::Application()
    : window(1600, 1200, "Draft"),
    windowPtr(window.getGlfwWindowPtr()),
    renderSystem(window),
    firstMouse(true),
    lastX(0.0f),
    lastY(0.0f),
    deltaTime(0.0f),
    lastFrame(0.0f),
    scene(),
    movementSystem(scene),
    worldSpaceInputSystem(scene),
    tankInputSystem(scene),
    noClipInputSystem(scene),
    patrolSystem(scene),
    collisionSystem(scene),
    inputDirection(glm::vec3(0.0f, 0.0f, 0.0f))
{
}

int Application::run() {
    while (!glfwWindowShouldClose(windowPtr)) {
        updateTiming();

        handleKeyInput();
        window.processEvents();

        Event event;
        while (window.pollEvent(event)) {
            handleWindowEvent(event);
        }

        // Fetch current camera
        if (scene.cameraSet.getEntities().empty()) {
            std::cerr << "[ERROR] No camera found in scene!\n";
            return -1;
        }

        uint32_t camEntity = scene.cameraSet.getEntities()[0];
        CameraComponent& camera = scene.cameraSet.getComponent(camEntity);
        // Could be optimised
        updateProjectionMatrix(camera, window.width, window.height);
        updateViewMatrix(camera);

        // Update systems
        worldSpaceInputSystem.updateVelocity(inputDirection);
        tankInputSystem.updateVelocity(inputDirection, deltaTime);
        noClipInputSystem.updateVelocity(inputDirection, camera.front, camera.right);
        patrolSystem.updateVelocity(deltaTime);
        collisionSystem.updateVelocity(deltaTime);
        movementSystem.updateTransforms(deltaTime);
        renderSystem.renderScene(scene);

        glfwSwapBuffers(windowPtr);
    }
    return 0;
}


void Application::handleWindowEvent(const Event& event) {
    if (scene.cameraSet.getEntities().empty()) return;
    CameraComponent& camera = scene.cameraSet.getComponent(scene.cameraSet.getEntities()[0]);

    switch (event.type) {
    case EventType::WindowResize: {
        glViewport(0, 0, event.resize.width, event.resize.height);
        window.width = event.resize.width;
        window.height = event.resize.height;
        updateProjectionMatrix(camera, window.width, window.height);
        break;
    }

    case EventType::MouseMove: {
        if (firstMouse) {
            lastX = event.mouseMove.xPos;
            lastY = event.mouseMove.yPos;
            firstMouse = false;
        }

        float xoffset = event.mouseMove.xPos - lastX;
        float yoffset = lastY - event.mouseMove.yPos;

        lastX = event.mouseMove.xPos;
        lastY = event.mouseMove.yPos;

        processMouseMovement(camera, xoffset, yoffset);
        break;
    }
    case EventType::Scroll: {
        processMouseScroll(camera, event.scroll.yOffset);
        break;
    }
    }
}

void Application::handleKeyInput() {
    inputDirection.direction = glm::vec3(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(windowPtr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(windowPtr, true);
    if (glfwGetKey(windowPtr, GLFW_KEY_W) == GLFW_PRESS)
        inputDirection.direction += glm::vec3(0.0f, 0.0f, -1.0f);
    if (glfwGetKey(windowPtr, GLFW_KEY_S) == GLFW_PRESS)
        inputDirection.direction += glm::vec3(0.0f, 0.0f, 1.0f);
    if (glfwGetKey(windowPtr, GLFW_KEY_A) == GLFW_PRESS)
        inputDirection.direction += glm::vec3(-1.0f, 0.0f, 0.0f);
    if (glfwGetKey(windowPtr, GLFW_KEY_D) == GLFW_PRESS)
        inputDirection.direction += glm::vec3(1.0f, 0.0f, 0.0f);
}

void Application::updateTiming() {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

int main() {
    Application app;
    return app.run();
}
