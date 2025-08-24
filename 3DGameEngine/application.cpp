#include "Application.h"
#include <iostream>


Application::Application()
    : window(1600, 1200, "Draft"),
     windowPtr(window.getGlfwWindowPtr()),
    renderSystem(window),
    camera(glm::vec3(0.0f, 0.0f, 3.0f)),
    firstMouse(true),
    lastX(0.0f),
    lastY(0.0f),
    deltaTime(0.0f),
    lastFrame(0.0f)
{
    Entity skyboxEntity{ .id = 0 };
    scene.entitiesInScene.push_back(skyboxEntity);
    scene.skyboxesInScene[skyboxEntity.id].isSkybox = true;
    Materials materials;
    Meshes meshes;
    scene.materialsInScene[skyboxEntity.id] = materials.materials["skybox"];
    scene.meshesInScene[skyboxEntity.id] = meshes.meshes["skybox"];

    Entity cubeEntity{ .id = 1 };
    scene.entitiesInScene.push_back(cubeEntity);
    scene.materialsInScene[cubeEntity.id] = materials.materials["cube"];
    scene.meshesInScene[cubeEntity.id] = meshes.meshes["cube"];
    glm::mat4 model = glm::mat4(1.0f);
    scene.updateTransforms(cubeEntity.id, model);
    materials.materials["cube"].shader->setMat3Uniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));

    Entity baseplateEntity{ .id = 2 };
    scene.entitiesInScene.push_back(baseplateEntity);
    scene.meshesInScene[baseplateEntity.id] = meshes.meshes["baseplate"];
    scene.materialsInScene[baseplateEntity.id] = materials.materials["simple"];
    scene.updateTransforms(baseplateEntity.id, model);
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
        camera.updateViewMatrix();
        camera.updateProjectionMatrix(window.width, window.height);
        renderSystem.renderScene(camera, scene);
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

        camera.ProcessMouseMovement(xoffset, yoffset);
        break;
    }
    case EventType::Scroll: {
        camera.ProcessMouseScroll(event.scroll.yOffset);
        break;
    }
    }
}

void Application::handleKeyInput() {
    if (glfwGetKey(windowPtr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(windowPtr, true);
    if (glfwGetKey(windowPtr, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(windowPtr, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(windowPtr, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(windowPtr, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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



/*
void processInput(GLFWwindow* window, RenderState& state)
{
    static bool eWasPressed = false;
    bool eIsPressed = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        state.camera.ProcessKeyboard(FORWARD, state.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        state.camera.ProcessKeyboard(BACKWARD, state.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        state.camera.ProcessKeyboard(LEFT, state.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        state.camera.ProcessKeyboard(RIGHT, state.deltaTime);
    if (eIsPressed && !eWasPressed) {
        state.showFPS = !state.showFPS;
    }
    eWasPressed = eIsPressed;
}
*/