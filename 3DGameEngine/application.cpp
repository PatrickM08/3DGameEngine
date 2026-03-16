#include "application.h"
#include "ecs.h"
#include "editor.h"
#include "events.h"
#include "window.h"
#include "serialization.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>

static constexpr bool LOAD_SCENE_FROM_FILE = false;
static constexpr const char* SCENE_PATH = "scene.bin";

int run(ECS& scene) {
    GLFWwindow* windowPtr = scene.window.windowPtr;
    glfwSetWindowUserPointer(windowPtr, &scene);
    glfwSetFramebufferSizeCallback(windowPtr, [](GLFWwindow* win, int w, int h) {
        ECS* scene = static_cast<ECS*>(glfwGetWindowUserPointer(win));
        pushEvent(scene->eventQueue, Event{.type = EventType::WindowResize, .resize = {w, h}});
    });
    glfwSetCursorPosCallback(windowPtr, [](GLFWwindow* win, double xPos, double yPos) {
        if (ImGui::GetIO().WantCaptureMouse) return;
        ECS* scene = static_cast<ECS*>(glfwGetWindowUserPointer(win));
        pushEvent(scene->eventQueue, Event{.type = EventType::MouseMove, .mouseMove = {(float)xPos, (float)yPos}});
    });
    glfwSetMouseButtonCallback(windowPtr, [](GLFWwindow* win, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
    });
    glfwSetScrollCallback(windowPtr, [](GLFWwindow* win, double xOffset, double yOffset) {
        ImGui_ImplGlfw_ScrollCallback(win, xOffset, yOffset);
        if (ImGui::GetIO().WantCaptureMouse) return;
        ECS* scene = static_cast<ECS*>(glfwGetWindowUserPointer(win));
        pushEvent(scene->eventQueue, Event{.type = EventType::Scroll, .scroll = {(float)yOffset}});
    });
    glfwSetCharCallback(windowPtr, [](GLFWwindow* win, unsigned int codepoint) {
        ImGui_ImplGlfw_CharCallback(win, codepoint);
    });
    glfwSetKeyCallback(windowPtr, keyCallback);


    if (LOAD_SCENE_FROM_FILE) loadScene(scene, SCENE_PATH);
    else initScene(scene);

    while (!glfwWindowShouldClose(windowPtr)) {
        setGui(scene);

        if (!scene.cameraSet.hasComponent(scene.currentCamera) && scene.cameraSet.entityCount > 0)
            scene.currentCamera = scene.cameraSet.entities[0];
        uint32_t camEntity = scene.currentCamera;
        CameraComponent& camera = scene.cameraSet.getComponent(camEntity);

        scene.physicsManifold.size = 0;
        scene.deleteBuffer.size = 0;

        updateTiming(scene);

        glfwPollEvents();
        

        Event event;
        while (pollEvent(scene.eventQueue, event)) {
            handleWindowEvent(event, scene.window, scene.framebuffer, camera, scene.mouseData);
        }

        processMouseMovement(camera, scene.mouseData.frameOffsetX, scene.mouseData.frameOffsetY, true);
        scene.mouseData.frameOffsetX = 0.0f;
        scene.mouseData.frameOffsetY = 0.0f;

        // Could be optimised
        updateProjectionMatrix(camera, scene.window.width, scene.window.height);
        updateCameraPosition(scene.transformSet, scene.cameraSet);
        updateViewMatrix(camera);

        // Update systems
        updateScene(scene, camera);

        drawGui(scene);

        glfwSwapBuffers(windowPtr);
        std::memcpy(scene.lastKeyStateBuffer, scene.keyStateBuffer, sizeof(scene.keyStateBuffer));
    }
    return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ECS* scene = static_cast<ECS*>(glfwGetWindowUserPointer(window));
    if (scene->debugMode) {
        scene->lastPressedGLFWKey = key;
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        break;
    // TODO: FIND A NICE WAY TO DO THIS
    case GLFW_KEY_A:
    case GLFW_KEY_B:
    case GLFW_KEY_C:
    case GLFW_KEY_D:
    case GLFW_KEY_E:
    case GLFW_KEY_F:
    case GLFW_KEY_G:
    case GLFW_KEY_H:
    case GLFW_KEY_I:
    case GLFW_KEY_J:
    case GLFW_KEY_K:
    case GLFW_KEY_L:
    case GLFW_KEY_M:
    case GLFW_KEY_N:
    case GLFW_KEY_O:
    case GLFW_KEY_P:
    case GLFW_KEY_R:
    case GLFW_KEY_S:
    case GLFW_KEY_T:
    case GLFW_KEY_U:
    case GLFW_KEY_V:
    case GLFW_KEY_W:
    case GLFW_KEY_X:
    case GLFW_KEY_Y:
    case GLFW_KEY_Z:
    case GLFW_KEY_0:
    case GLFW_KEY_1:
    case GLFW_KEY_2:
    case GLFW_KEY_3:
    case GLFW_KEY_4:
    case GLFW_KEY_5:
    case GLFW_KEY_6:
    case GLFW_KEY_7:
    case GLFW_KEY_8:
    case GLFW_KEY_9:
    case GLFW_KEY_SPACE:
        switch (action) {
        case GLFW_PRESS:
            scene->keyStateBuffer[key - 31] = 1.0f;
            break;
        case GLFW_RELEASE:
            scene->keyStateBuffer[key - 31] = 0.0f;
            break;
        case GLFW_REPEAT:
            break;
        // TODO: MAYBE ADD AN UNREACHABLE FLAG HERE
        }
        break;
    case GLFW_KEY_Q:
        switch (action) {
        case GLFW_PRESS:
            scene->debugMode ^= 1;
            break;
        case GLFW_RELEASE:
            break;
        case GLFW_REPEAT:
            break;
        }
        break;
    }
}


int main() {
    try {
        ECS* scene = new ECS();
        initState(*scene);
        return run(*scene);
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Fatal Error." << std::endl;
        return -1;
    }
}


