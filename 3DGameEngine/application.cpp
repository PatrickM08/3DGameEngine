#include "application.h"
#include "ecs.h"
#include "movement_system.h"
#include "collision_system.h"
#include "render_system.h"
#include <iostream>
#include <chrono>


// THIS SHOULD BE HOW THE STATE IS USED NO INITIALISATION OF STATE OR MODIFICATION OF STATE

int run(Application& app) {
    ECS& scene = *app.scene;
    GLFWwindow* windowPtr = scene.window.windowPtr;
    glfwSetWindowUserPointer(windowPtr, &scene);
    glfwSetFramebufferSizeCallback(windowPtr, [](GLFWwindow* win, int w, int h) {
        ECS* scene = static_cast<ECS*>(glfwGetWindowUserPointer(win));
        pushEvent(scene->eventQueue, Event{.type = EventType::WindowResize, .resize = {w, h}});
    });
    glfwSetCursorPosCallback(windowPtr, [](GLFWwindow* win, double xPos, double yPos) {
        ECS* scene = static_cast<ECS*>(glfwGetWindowUserPointer(win));
        pushEvent(scene->eventQueue, Event{.type = EventType::MouseMove, .mouseMove = {(float)xPos, (float)yPos}});
    });
    glfwSetScrollCallback(windowPtr, [](GLFWwindow* win, double xOffset, double yOffset) {
        ECS* scene = static_cast<ECS*>(glfwGetWindowUserPointer(win));
        pushEvent(scene->eventQueue, Event{.type = EventType::Scroll, .scroll = {(float)yOffset}});
    });
    glfwSetKeyCallback(windowPtr, keyCallback);
    initScene(scene);


    while (!glfwWindowShouldClose(windowPtr)) {
        //auto start = std::chrono::steady_clock::now();

        // Fetch current camera TODO: CHANGE THIS
        if (scene.cameraSet.entityCount == 0) {
            std::cerr << "[ERROR] No camera found in scene!\n";
            return -1;
        }

        uint32_t camEntity = scene.cameraSet.entities[0];
        CameraComponent& camera = scene.cameraSet.getComponent(camEntity);

        // TODO: FIND THE CORRECT PLACE TO CLEAR BUFFERS
        scene.physicsManifold.size = 0;
        scene.deleteBuffer.size = 0;

        updateTiming(app);

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
        updateViewMatrix(camera);

        // Update systems
        worldSpaceInputSystem(scene.inputWorldSet, scene.velocitySet, scene.speedSet, scene.inputMapSet, scene.keyStateBuffer);
        tankInputSystem(scene.rotationSpeedSet, scene.speedSet, scene.inputTankSet, 
                        scene.velocitySet, scene.transformSet, app.deltaTime, scene.keyStateBuffer, scene.inputMapSet);
        noClipInputSystem(scene.inputNoClipSet, scene.speedSet, scene.velocitySet, scene.inputMapSet, scene.keyStateBuffer, camera.front,camera.right);
        patrolSystem(scene.patrolSet, scene.speedSet, scene.velocitySet, app.deltaTime);
        movementSystem(scene.velocitySet, scene.transformSet, scene.cameraSet, app.deltaTime);
        bulletSystem(scene);
        collisionSystem(scene.collisionSet, scene.transformSet, scene.dynamicSet, scene.bulletSet, scene.healthSet,
                        scene.physicsManifold, scene.deleteBuffer);
        resolveCollisions(scene.physicsManifold, scene.transformSet, scene.velocitySet);
        healthSystem(scene.healthSet, scene.deleteBuffer);
        performLightCulling(scene.pointLightSet, scene.transformSet, scene.visiblePointLights, scene.cameraSet.dense[0].frustumPlanes);
        uploadLightSSBO(scene.lightSSBO, scene.visiblePointLights);
        updateSceneData(scene.sceneData, scene.cameraSet.dense[0], scene.visiblePointLights, scene.skyboxData);
        uploadSceneUBO(scene.sceneUBO, scene.sceneData);
        performFrustumCulling(scene.renderableSet, scene.transformSet, scene.meshSet, 
                              scene.visibleEntities, scene.cameraSet.dense[0].frustumPlanes);
        renderSystem(scene.visibleEntities, scene.materialSet, scene.meshSet, scene.transformSet, scene.framebuffer);
        renderSkybox(scene.skyboxData);
        drawToFramebuffer(scene.framebuffer, scene.quadVAO);
        glDisable(GL_DEPTH_TEST);
        std::string fps = "FPS: " + std::to_string(int(1 / app.deltaTime));
        renderText(fps.c_str(), 10, 20, 20, 1, scene.window.width, scene.window.height, scene.textRenderData);
        glEnable(GL_DEPTH_TEST);
        //auto end = std::chrono::steady_clock::now();
        glfwSwapBuffers(windowPtr);
        deleteSystem(scene);
        std::memcpy(scene.lastKeyStateBuffer, scene.keyStateBuffer, sizeof(scene.keyStateBuffer));
    }
    return 0;
}

void updateTiming(Application& app) {
    float currentFrame = (float)glfwGetTime();
    app.deltaTime = currentFrame - app.lastFrame;
    app.lastFrame = currentFrame;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ECS* scene = static_cast<ECS*>(glfwGetWindowUserPointer(window));
    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        break;
    // TODO: FIND A NICE WAY TO DO THIS
    case GLFW_KEY_W:
    case GLFW_KEY_S:
    case GLFW_KEY_A:
    case GLFW_KEY_D:
    case GLFW_KEY_I:
    case GLFW_KEY_K:
    case GLFW_KEY_J:
    case GLFW_KEY_L:
    case GLFW_KEY_SPACE:
    case GLFW_KEY_M:
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
    }
}


int main() {
    try {
        Application app;
        initState(*app.scene);
        return run(app);
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Fatal Error." << std::endl;
        return -1;
    }
}
