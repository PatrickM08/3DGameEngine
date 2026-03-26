#define _CRT_SECURE_NO_WARNINGS
#include "ecs.h"
#include "movement_system.h"
#include "collision_system.h"
#include "render_system.h"
#include "shader_s.h"
#include "window.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include "asset_manager.h"
#include <chrono>
#include <iostream>

uint32_t createCube(ECS& scene, glm::vec3 pos, glm::vec3 scale, uint32_t materialID) {
    uint32_t id = createEntity(scene);

    scene.meshSet.add(id, scene.meshBuffer.buffer[scene.cubePrimitiveIndex]);
    scene.materialSet.add(id, scene.materialBuffer.buffer[materialID]);
    scene.renderableSet.add(id, RenderableTag{});
    scene.velocitySet.add(id, VelocityComponent{glm::vec3(0.0f)});
    scene.transformSet.add(id, TransformComponent{.position = pos, .scale = scale});

    glm::vec3 half = scale * 0.5f;
    scene.collisionSet.add(id, CollisionComponent{
                                   .minX = -half.x, .maxX = half.x, .minY = -half.y, .maxY = half.y, .minZ = -half.z, .maxZ = half.z});

    return id;
}

void initState(ECS& scene) {
    scene.arena.init(ARENA_SIZE);
    scene.transformSet.init(scene.arena, CAPACITY_TRANSFORM);
    scene.meshSet.init(scene.arena, CAPACITY_MESH);
    scene.materialSet.init(scene.arena, CAPACITY_MATERIAL);
    scene.inputWorldSet.init(scene.arena, CAPACITY_INPUT_WORLD);
    scene.inputTankSet.init(scene.arena, CAPACITY_INPUT_TANK);
    scene.inputNoClipSet.init(scene.arena, CAPACITY_INPUT_NOCLIP);
    scene.velocitySet.init(scene.arena, CAPACITY_VELOCITY);
    scene.speedSet.init(scene.arena, CAPACITY_SPEED);
    scene.rotationSpeedSet.init(scene.arena, CAPACITY_ROT_SPEED);
    scene.patrolSet.init(scene.arena, CAPACITY_PATROL);
    scene.collisionSet.init(scene.arena, CAPACITY_COLLISION);
    scene.renderableSet.init(scene.arena, CAPACITY_RENDERABLE);
    scene.cameraSet.init(scene.arena, CAPACITY_CAMERA);
    scene.pointLightSet.init(scene.arena, CAPACITY_POINT_LIGHT);
    scene.bulletSet.init(scene.arena, CAPACITY_BULLET);
    scene.dynamicSet.init(scene.arena, CAPACITY_DYNAMIC);
    scene.healthSet.init(scene.arena, CAPACITY_HEALTH);
    scene.inputMapSet.init(scene.arena, CAPACITY_INPUT_MAP);
    scene.nameSet.init(scene.arena, CAPACITY_NAME);

    scene.window.width = 1600;
    scene.window.height = 1200;
    scene.window.title = "PROTOPLAY";
    scene.window.windowPtr = createWindow(scene.window.width, scene.window.height, scene.window.title);
    initOpenglRenderState();
    initDefaultMaterials(scene.materialBuffer, scene.materialSSBODataBuffer);
    scene.materialSSBO = initMaterialSSBO(scene.materialSSBODataBuffer);
    scene.cubePrimitiveIndex = createUnitCubePrimitive(scene.meshBuffer);
    initMeshes(scene.meshBuffer);
    scene.framebuffer = createFrameBuffer(createShaderProgram("fb_vertex_shader.vs", "fb_fragment_shader.fs"), scene.window.width, scene.window.height);
    scene.quadVAO = createQuad();
    scene.lightSSBO = createLightSSBO(scene.visiblePointLightBuffer.capacity);
    scene.skyboxData.cubemapHandle = loadSkyboxCubemap();
    scene.skyboxData.shaderID = createShaderProgram("skybox.vs", "skybox.fs");
    scene.skyboxData.meshVAO = scene.meshBuffer.buffer[3].vao; // TODO: CHANGE
    scene.sceneUBO = createSceneUBO();
    std::memset(scene.keyStateBuffer, 0, sizeof(scene.keyStateBuffer));
    std::memset(scene.lastKeyStateBuffer, 0, sizeof(scene.lastKeyStateBuffer));
    TextRenderData& textRenderData = scene.textRenderData;
    setupTextBuffers(textRenderData.textVAO, textRenderData.textVBO);
    textRenderData.textShaderID = createShaderProgram("text_vertex_shader.vs", "text_fragment_shader.fs");
    parseFont("ariallatin.fnt", textRenderData.glyphs, textRenderData.glyphCount);
    textRenderData.bitmapFontTextureID = loadBitmapFont("ariallatin_0.png", textRenderData.glyphs, textRenderData.glyphCount);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 2.0f;
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(scene.window.windowPtr, false);
    ImGui_ImplOpenGL3_Init("#version 430");
}

void initScene(ECS& scene) {
    uint32_t camEntity = createEntity(scene);
    scene.meshSet.add(camEntity, scene.meshBuffer.buffer[1]);
    scene.velocitySet.add(camEntity, VelocityComponent{glm::vec3(0.0f)});
    scene.speedSet.add(camEntity, SpeedComponent{5.0f});
    scene.transformSet.add(camEntity, TransformComponent{.position = glm::vec3(0.0f, 0.0f, 0.0f)});
    scene.inputNoClipSet.add(camEntity, PlayerInputNoClipTag{});
    scene.inputMapSet.add(camEntity, {.forwardIndex = GLFW_KEY_W - 31, .backIndex = GLFW_KEY_S - 31, .leftIndex = GLFW_KEY_A - 31, .rightIndex = GLFW_KEY_D - 31, .shootIndex = GLFW_KEY_SPACE - 31});
    CameraComponent camera{
        .positionOffset = glm::vec3(0.0f),
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
        .worldUp = glm::vec3(0, 1, 0),
        .yaw = -45.0f,
        .pitch = -90.0f,
        .cameraType = CameraType::MOUSETURN};
    updateCameraVectors(camera);
    scene.cameraSet.add(camEntity, camera);
    scene.nameSet.add(camEntity, NameComponent{"Camera"});

    const uint32_t numEntities = 5000;
    const uint32_t numLights = 100;
    const uint32_t lightEvery = numEntities / numLights;

    for (uint32_t i = 0; i < numEntities; ++i) {
        float xPos = static_cast<float>(i % 70) * 3.0f;
        float zPos = static_cast<float>(i / 70) * 3.0f;

        uint32_t e = createCube(scene, {xPos, 0.5f, zPos}, {1, 1, 1}, 0);

        if (i % lightEvery == 0) {
            glm::vec3 colour;
            int pattern = (i / lightEvery) % 3;
            if (pattern == 0) colour = {1.0f, 0.2f, 0.2f};
            else if (pattern == 1) colour = {0.2f, 1.0f, 0.2f};
            else colour = {0.2f, 0.2f, 1.0f};

            uint32_t light = createCube(scene, {xPos, 5.5f, zPos}, {0.3f, 0.3f, 0.3f}, 0);
            scene.pointLightSet.add(light, PointLightComponent{
                .colour = colour,
                .intensity = 1.0f,
                .radius = 15.0f});
        }
    }
}

void addTextFloat(ECS& scene, const char* text, float value, uint8_t textLength, float xPos, float yPos, float size) {
    if (scene.textBuffer.size == scene.textBuffer.capacity) return;
    TextEntry textEntry;
    snprintf(textEntry.text, sizeof(textEntry.text), "%s%f", text, value);
    textEntry.textLength = textLength;
    textEntry.xPos = xPos;
    textEntry.yPos = yPos;
    textEntry.size = size;
    scene.textBuffer.buffer[scene.textBuffer.size++] = textEntry;
}

void updateScene(ECS& scene, CameraComponent& camera) {
    worldSpaceInputSystem(scene.inputWorldSet, scene.velocitySet, scene.speedSet, scene.inputMapSet, scene.keyStateBuffer);
    tankInputSystem(scene.rotationSpeedSet, scene.speedSet, scene.inputTankSet,
                    scene.velocitySet, scene.transformSet, scene.deltaTime, scene.keyStateBuffer, scene.inputMapSet);
    noClipInputSystem(scene.inputNoClipSet, scene.speedSet, scene.velocitySet, scene.inputMapSet, scene.keyStateBuffer, camera.front, camera.right);
    patrolSystem(scene.patrolSet, scene.speedSet, scene.velocitySet, scene.deltaTime);
    movementSystem(scene.velocitySet, scene.transformSet, scene.deltaTime);
    bulletSystem(scene);
    collisionSystem(scene.collisionSet, scene.transformSet, scene.dynamicSet, scene.bulletSet, scene.healthSet,
                    scene.physicsManifold, scene.deleteBuffer);
    resolveCollisions(scene.physicsManifold, scene.transformSet, scene.velocitySet);

    healthSystem(scene.healthSet, scene.deleteBuffer);
    performLightCulling(scene.pointLightSet, scene.transformSet, scene.visiblePointLightBuffer, camera.frustumPlanes);
    uploadLightSSBO(scene.lightSSBO, scene.visiblePointLightBuffer);
    updateSceneData(scene.sceneData, camera, scene.visiblePointLightBuffer, scene.skyboxData);
    uploadSceneUBO(scene.sceneUBO, scene.sceneData);
    performFrustumCulling(scene.renderableSet, scene.transformSet, scene.meshSet,
                          scene.visibleEntityBuffer, camera.frustumPlanes);

    auto start = std::chrono::steady_clock::now();
    renderSystem(scene.visibleEntityBuffer, scene.materialSet, scene.meshSet, scene.transformSet, scene.framebuffer);
    renderSkybox(scene.skyboxData);
    drawToFramebuffer(scene.framebuffer, scene.quadVAO);
    auto end = std::chrono::steady_clock::now();

    //std::cout << end - start << std::endl;

    renderTextSystem(scene.textBuffer, scene.textRenderData, scene.window.width, scene.window.height);
    deleteSystem(scene);
}

void bulletSystem(ECS& scene) {
    SparseSet<InputMapComponent>& inputMapSet = scene.inputMapSet;
    float* keyStateBuffer = scene.keyStateBuffer;
    float* lastKeyStateBuffer = scene.lastKeyStateBuffer;
    SparseSet<TransformComponent>& transformSet = scene.transformSet;
    for (uint32_t i = 0; i < inputMapSet.entityCount; ++i) {
        uint32_t entity = inputMapSet.entities[i];
        const TransformComponent& transform = transformSet.getComponent(entity);
        const InputMapComponent& inputMap = inputMapSet.getComponent(entity);
        if (keyStateBuffer[inputMap.shootIndex] && !lastKeyStateBuffer[inputMap.shootIndex]) {
            createBullet(scene, transform.position, transform.rotation);
        }
    }
}

// TODO: NEED TO MAKE THIS BETTER
void createBullet(ECS& scene, glm::vec3 position, glm::quat rotation) {
    uint32_t bullet = createEntity(scene);
    scene.meshSet.add(bullet, scene.meshBuffer.buffer[scene.cubePrimitiveIndex]);
    scene.materialSet.add(bullet, scene.materialBuffer.buffer[0]);
    scene.renderableSet.add(bullet, RenderableTag{});
    glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    float bulletOffset = 1.0f;
    position += bulletOffset * front;
    scene.transformSet.add(bullet, TransformComponent{.position = position, .scale = glm::vec3(0.2f)});
    scene.collisionSet.add(bullet, CollisionComponent{.minX = -0.1f, .maxX = 0.1f, .minY = -0.1f, .maxY = 0.1f, .minZ = -0.1f, .maxZ = 0.1f});
    scene.velocitySet.add(bullet, VelocityComponent{glm::vec3(10.0f * front)});
    scene.bulletSet.add(bullet, BulletTag{});
    scene.dynamicSet.add(bullet, DynamicTag{});
}

void updateTiming(ECS& scene) {
    float currentFrame = (float)glfwGetTime();
    scene.deltaTime = currentFrame - scene.lastFrame;
    scene.lastFrame = currentFrame;
}

uint32_t createEntity(ECS& scene) {
    if (scene.freeStackSize > 0) {
        return scene.freeStack[--scene.freeStackSize];
    }
    return ++scene.entityCount;
}