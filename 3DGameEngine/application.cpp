#include "Application.h"
#include "camera.h"
#include "ecs.h"
#include "sparse_set.h"
#include <iostream>
#include <chrono>

Application::Application()
    : window(1600, 1200, "Draft"), windowPtr(window.getGlfwWindowPtr()),
      renderSystem(window), firstMouse(true), lastX(0.0f), lastY(0.0f),
      deltaTime(0.0f), lastFrame(0.0f), scene(),
      inputDirection(glm::vec3(0.0f, 0.0f, 0.0f)) {}

int Application::run() {
    init(scene);
    while (!glfwWindowShouldClose(windowPtr)) {
        auto start = std::chrono::steady_clock::now();
        // TODO: FIND THE CORRECT PLACE TO CLEAR BUFFERS
        scene.physicsManifold.size = 0;
        scene.deleteBuffer.size = 0;

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
        worldSpaceInputSystem(scene.inputWorldSet, scene.velocitySet, scene.speedSet, inputDirection);
        tankInputSystem(scene.rotationSpeedSet, scene.speedSet, scene.inputTankSet, 
                        scene.velocitySet, scene.transformSet, inputDirection, deltaTime);
        noClipInputSystem(scene.inputNoClipSet, scene.speedSet, scene.velocitySet, inputDirection, camera.front,camera.right);
        patrolSystem(scene.patrolSet, scene.speedSet, scene.velocitySet, deltaTime);
        movementSystem(scene.velocitySet, scene.transformSet, scene.cameraSet, deltaTime);
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
        renderSystem.renderScene(scene.visibleEntities, scene.materialSet, scene.meshSet, scene.transformSet);
        renderSkybox(scene.skyboxData);
        renderSystem.drawToFramebuffer();
        auto end = std::chrono::steady_clock::now();
        glfwSwapBuffers(windowPtr);
        deleteSystem(scene);
        std::cout << "all Systems: " << end - start << "\n";
    }
    return 0;
}

void Application::handleWindowEvent(const Event& event) {
    if (scene.cameraSet.getEntities().empty())
        return;
    CameraComponent& camera =
        scene.cameraSet.getComponent(scene.cameraSet.getEntities()[0]);

    switch (event.type) {
    case EventType::WindowResize: {
        glViewport(0, 0, event.resize.width, event.resize.height);
        window.width = event.resize.width;
        window.height = event.resize.height;
        renderSystem.framebuffer = createFrameBuffer(
            renderSystem.framebufferShaderID, window.width, window.height);
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
    static bool spaceDown = false;
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
    // TODO: MAKE THIS A PROPER SYSTEM.
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

void Application::updateTiming() {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

int main() {
    try {
        Application app;
        return app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Fatal Error." << std::endl;
        return -1;
    }
}
