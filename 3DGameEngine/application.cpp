#include "Application.h"
#include "asset_manager.h"
#include "render_system.h"
#include "ecs.h"
#include "movement_system.h"
#include "camera.h"
#include <iostream>



Application::Application()
    : window(1600, 1200, "Draft"),
    windowPtr(window.getGlfwWindowPtr()),
    renderSystem(window),
    firstMouse(true),
    lastX(0.0f),
    lastY(0.0f),
    deltaTime(0.0f),
    lastFrame(0.0f),
    scene(assetManager),
    movementSystem(scene),
    worldSpaceInputSystem(scene),
    tankInputSystem(scene),
    patrolSystem(scene),
    inputDirection(glm::vec3(0.0f, 0.0f, 0.0f))
{
    scene.camera.updateProjectionMatrix(window.width, window.height);
}

int Application::run() {
    while (!glfwWindowShouldClose(windowPtr)) {
        updateTiming();

        handleKeyInput();
        window.processEvents();
        Event event;
        while (window.pollEvent(event)) {
            handleEvent(event);
        }
        scene.camera.updateViewMatrix();

        worldSpaceInputSystem.updateVelocity(inputDirection);
        tankInputSystem.updateVelocity(inputDirection, deltaTime);
        patrolSystem.updateVelocity(deltaTime);
        movementSystem.updateTransforms(deltaTime);
        renderSystem.renderScene(scene);
        glfwSwapBuffers(windowPtr);
    }
    return 0;
}

void Application::handleEvent(const Event& event) {
    switch (event.type) {
    case EventType::WindowResize: {
        glViewport(0, 0, event.resize.width, event.resize.height);
        window.width = event.resize.width;
        window.height = event.resize.height;
        scene.camera.updateProjectionMatrix(window.width, window.height);
        break;
    }

    case EventType::MouseMove: {
        if (firstMouse)
        {
            lastX = event.mouseMove.xPos;
            lastY = event.mouseMove.yPos;
            firstMouse = false;
        }

        float xoffset = event.mouseMove.xPos - lastX;
        float yoffset = lastY - event.mouseMove.yPos;

        lastX = event.mouseMove.xPos;
        lastY = event.mouseMove.yPos;

        scene.camera.ProcessMouseMovement(xoffset, yoffset);
        break;
    }
    case EventType::Scroll: {
        scene.camera.ProcessMouseScroll(event.scroll.yOffset);
        break;
    }
    }
}

void Application::handleKeyInput() {
    inputDirection.direction = glm::vec3(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(windowPtr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(windowPtr, true);
    if (glfwGetKey(windowPtr, GLFW_KEY_W) == GLFW_PRESS) {
        scene.camera.ProcessKeyboard(FORWARD, deltaTime);
        inputDirection.direction += glm::vec3(0.0f, 0.0f, -1.0f);
    }
    if (glfwGetKey(windowPtr, GLFW_KEY_S) == GLFW_PRESS) {
        scene.camera.ProcessKeyboard(BACKWARD, deltaTime);
        inputDirection.direction += glm::vec3(0.0f, 0.0f, 1.0f);
    }
    if (glfwGetKey(windowPtr, GLFW_KEY_A) == GLFW_PRESS) {
        scene.camera.ProcessKeyboard(LEFT, deltaTime);
        inputDirection.direction += glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (glfwGetKey(windowPtr, GLFW_KEY_D) == GLFW_PRESS) {
        scene.camera.ProcessKeyboard(RIGHT, deltaTime);
        inputDirection.direction += glm::vec3(1.0f, 0.0f, 0.0f);
    }
}

void Application::updateTiming() {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

int main() {
    Application app;
    app.run();
}
