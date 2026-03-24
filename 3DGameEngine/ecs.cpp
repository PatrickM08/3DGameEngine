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
    initMeshes("meshes.txt", scene.meshBuffer);
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
    scene.meshSet.add(scene.entityCount, scene.meshBuffer.buffer[1]);
    scene.transformSet.add(scene.entityCount, TransformComponent{.position = glm::vec3(0.0f, 20.0f, 10.0f)});

    CameraComponent camera{
        .positionOffset = glm::vec3(0.0f),
        .worldUp = glm::vec3(0, 1, 0),
        .yaw = -90.0f,
        .pitch = -60.0f,
        .cameraType = CameraType::FIXED};

    updateCameraVectors(camera);
    scene.cameraSet.add(scene.entityCount, camera);
    scene.nameSet.add(scene.entityCount, NameComponent{"Camera"});

    uint32_t baseplate = createEntity(scene);
    scene.meshSet.add(baseplate, scene.meshBuffer.buffer[1]);
    scene.materialSet.add(baseplate, scene.materialBuffer.buffer[1]);
    scene.renderableSet.add(baseplate, RenderableTag{});
    scene.transformSet.add(baseplate, TransformComponent{.position = glm::vec3(-50, 0, 50)});
    scene.nameSet.add(baseplate, NameComponent{"Baseplate"});

    const uint32_t MAT = 0;

    uint32_t p1 = createCube(scene, {-5.0f, 0.5f, -2.0f}, {1, 1, 1}, MAT);
    scene.speedSet.add(p1, {5.0f});
    scene.rotationSpeedSet.add(p1, {100.0f});
    scene.inputTankSet.add(p1, PlayerInputTankTag{});
    scene.inputMapSet.add(p1, {.forwardIndex = GLFW_KEY_W - 31, .backIndex = GLFW_KEY_S - 31, .leftIndex = GLFW_KEY_A - 31, .rightIndex = GLFW_KEY_D - 31, .shootIndex = GLFW_KEY_SPACE - 31});
    scene.dynamicSet.add(p1, DynamicTag{});
    scene.healthSet.add(p1, {3});
    scene.nameSet.add(p1, NameComponent{"Player 1"});

    uint32_t p2 = createCube(scene, {5.0f, 0.5f, -2.0f}, {1, 1, 1}, MAT);
    scene.speedSet.add(p2, {5.0f});
    scene.rotationSpeedSet.add(p2, {100.0f});
    scene.inputTankSet.add(p2, PlayerInputTankTag{});
    scene.inputMapSet.add(p2, {.forwardIndex = GLFW_KEY_I - 31, .backIndex = GLFW_KEY_K - 31, .leftIndex = GLFW_KEY_J - 31, .rightIndex = GLFW_KEY_L - 31, .shootIndex = GLFW_KEY_M - 31});
    scene.dynamicSet.add(p2, DynamicTag{});
    scene.healthSet.add(p2, {3});
    scene.nameSet.add(p2, NameComponent{"Player 2"});

    uint32_t l1 = createCube(scene, {0.0f, 0.5f, -4.0f}, {1, 1, 1}, MAT);
    scene.pointLightSet.add(l1, {.colour = {1.0f, 1.0f, 1.0f}, .intensity = 1.0f, .radius = 10.0f});
    scene.nameSet.add(l1, NameComponent{"Light 1"});

    uint32_t l2 = createCube(scene, {-3.0f, 0.5f, 0.0f}, {1, 1, 1}, MAT);
    scene.pointLightSet.add(l2, {.colour = {1.0f, 1.0f, 1.0f}, .intensity = 1.0f, .radius = 10.0f});
    scene.nameSet.add(l2, NameComponent{"Light 2"});

    createCube(scene, {0.0f, 0.5f, -10.0f}, {20.0f, 1.0f, 1.0f}, MAT);
    createCube(scene, {0.0f, 0.5f, 2.0f}, {20.0f, 1.0f, 1.0f}, MAT);
    createCube(scene, {-10.0f, 0.5f, -4.0f}, {1.0f, 1.0f, 12.0f}, MAT);
    createCube(scene, {10.0f, 0.5f, -4.0f}, {1.0f, 1.0f, 12.0f}, MAT);
}

void addText(ECS& scene, const char* text, uint8_t textLength, float xPos, float yPos, float size) {
    if (scene.textBuffer.size == scene.textBuffer.capacity) return;
    TextEntry textEntry;
    snprintf(textEntry.text, sizeof(textEntry.text), "%s", text);
    textEntry.textLength = textLength;
    textEntry.xPos = xPos;
    textEntry.yPos = yPos;
    textEntry.size = size;
    scene.textBuffer.buffer[scene.textBuffer.size++] = textEntry;
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