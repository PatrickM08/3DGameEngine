#include "application.h"
#include "ecs.h"
#include "movement_system.h"
#include "collision_system.h"
#include "render_system.h"
#include <iostream>
#include <chrono>


// THIS SHOULD BE HOW THE STATE IS USED NO INITIALISATION OF STATE OR MODIFICATION OF STATE

int run(Application& app) {
    ECS& scene = app.scene;
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
    init(scene);
    while (!glfwWindowShouldClose(windowPtr)) {
        auto start = std::chrono::steady_clock::now();

        // Fetch current camera TODO: CHANGE THIS
        if (scene.cameraSet.getEntities().empty()) {
            std::cerr << "[ERROR] No camera found in scene!\n";
            return -1;
        }

        uint32_t camEntity = scene.cameraSet.getEntities()[0];
        CameraComponent& camera = scene.cameraSet.getComponent(camEntity);

        // TODO: FIND THE CORRECT PLACE TO CLEAR BUFFERS
        scene.physicsManifold.size = 0;
        scene.deleteBuffer.size = 0;

        updateTiming(app);

        handleKeyInput(scene);
        glfwPollEvents();

        Event event;
        while (pollEvent(scene.eventQueue, event)) {
            handleWindowEvent(event, scene.window, scene.framebuffer, camera, scene.mouseData);
        }

        // Could be optimised
        updateProjectionMatrix(camera, scene.window.width, scene.window.height);
        updateViewMatrix(camera);

        // Update systems
        worldSpaceInputSystem(scene.inputWorldSet, scene.velocitySet, scene.speedSet, scene.keyStateBuffer);
        //tankInputSystem(scene.rotationSpeedSet, scene.speedSet, scene.inputTankSet, 
          //              scene.velocitySet, scene.transformSet, inputDirection, deltaTime);
        //noClipInputSystem(scene.inputNoClipSet, scene.speedSet, scene.velocitySet, inputDirection, camera.front,camera.right);
        patrolSystem(scene.patrolSet, scene.speedSet, scene.velocitySet, app.deltaTime);
        movementSystem(scene.velocitySet, scene.transformSet, scene.cameraSet, app.deltaTime);
        collisionSystem(scene.collisionSet, scene.transformSet, scene.dynamicSet, scene.bulletSet, scene.healthSet,
                        scene.physicsManifold, scene.deleteBuffer);
        resolveCollisions(scene.physicsManifold, scene.transformSet, scene.velocitySet);
        healthSystem(scene.healthSet, scene.deleteBuffer);
        performLightCulling(scene.pointLightSet, scene.transformSet, scene.visiblePointLights, scene.cameraSet.dense[0].frustumPlanes);
        uploadLightSSBO(scene.lightSSBO, scene.visiblePointLights);
        updateSceneData(scene.sceneData, scene.cameraSet.dense[0], scene.visiblePointLights, scene.skyboxData);
        uploadSceneUBO(scene.sceneUBO, scene.sceneData);
        performFrustumCulling(scene.renderableSet.entities, scene.transformSet, scene.meshSet, 
                              scene.visibleEntities, scene.cameraSet.dense[0].frustumPlanes);
        renderSystem(scene.visibleEntities, scene.materialSet, scene.meshSet, scene.transformSet, scene.framebuffer);
        renderSkybox(scene.skyboxData);
        drawToFramebuffer(scene.framebuffer, scene.quadVAO);
        auto end = std::chrono::steady_clock::now();
        glfwSwapBuffers(windowPtr);
        deleteSystem(scene);
        std::cout << "all Systems: " << end - start << "\n";
    }
    return 0;
}

void handleKeyInput(ECS& scene) {
    GLFWwindow* windowPtr = scene.window.windowPtr;
    std::fill(scene.keyStateBuffer, scene.keyStateBuffer + 316, false);
    static bool spaceDown = false;
    if (glfwGetKey(windowPtr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(windowPtr, true);
    if (glfwGetKey(windowPtr, GLFW_KEY_W) == GLFW_PRESS)
        scene.keyStateBuffer[GLFW_KEY_W - 32] = true;
    if (glfwGetKey(windowPtr, GLFW_KEY_S) == GLFW_PRESS)
        scene.keyStateBuffer[GLFW_KEY_S - 32] = true;
    if (glfwGetKey(windowPtr, GLFW_KEY_A) == GLFW_PRESS)
        scene.keyStateBuffer[GLFW_KEY_A - 32] = true;
    if (glfwGetKey(windowPtr, GLFW_KEY_D) == GLFW_PRESS)
        scene.keyStateBuffer[GLFW_KEY_D - 32] = true;
    // TODO: MAKE THIS A PROPER SYSTEM. We need a buffer with booleans that represenet key mappings (map an enum to an index <- an enum is an index)
    if (glfwGetKey(windowPtr, GLFW_KEY_SPACE) == GLFW_RELEASE)
        spaceDown = false;
    if (glfwGetKey(windowPtr, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceDown) {
        spaceDown = true;
        uint32_t player = scene.inputTankSet.entities[0];
        glm::vec3 position = scene.transformSet.getComponent(player).position;
        glm::quat rotation = scene.transformSet.getComponent(player).rotation;
        createBullet(scene, position, rotation);
    }
}

void updateTiming(Application& app) {
    float currentFrame = (float)glfwGetTime();
    app.deltaTime = currentFrame - app.lastFrame;
    app.lastFrame = currentFrame;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    return;
}


int main() {
    try {
        Application app;
        return run(app);
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Fatal Error." << std::endl;
        return -1;
    }
}
