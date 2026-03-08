#include "ecs.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "shader_s.h"
#include "camera.h"
#include "window.h"
#include "asset_manager.h"

uint32_t createCube(ECS& scene, glm::vec3 pos, glm::vec3 scale, uint32_t meshID, uint32_t materialID) {
    uint32_t id = ++scene.entityCount; // Automate this!

    scene.meshSet.add(id, scene.assetManager.meshes[meshID]);
    scene.materialSet.add(id, scene.assetManager.materials[materialID]);
    scene.renderableSet.add(id, RenderableTag{});
    scene.velocitySet.add(id, VelocityComponent{glm::vec3(0.0f)});
    scene.transformSet.add(id, TransformComponent{.position = pos, .scale = scale});

    // Automatically calculate collision bounds based on scale
    glm::vec3 half = scale * 0.5f;
    scene.collisionSet.add(id, CollisionComponent{
                                   .minX = -half.x, .maxX = half.x, .minY = -half.y, .maxY = half.y, .minZ = -half.z, .maxZ = half.z});

    return id;
}

void initState(ECS& scene) {
    scene.entityCount = 0;
    scene.window.width = 1600;
    scene.window.height = 1200;
    scene.window.title = "PROTOPLAY";
    scene.window.windowPtr = createWindow(scene.window.width, scene.window.height, scene.window.title);
    scene.assetManager.materials = loadMaterials();
    scene.assetManager.meshes = loadMeshes("meshes.txt");
    scene.framebuffer = createFrameBuffer(createShaderProgram("fb_vertex_shader.vs", "fb_fragment_shader.fs"), scene.window.width, scene.window.height);
    scene.quadVAO = createQuad();
    scene.visibleEntities.reserve(20000); // The maximum number of entities that can be rendered per pass.
    scene.visiblePointLights.reserve(256); // The maximum number of lights that can influence the view frustum.
    scene.lightSSBO = createLightSSBO();
    scene.skyboxData.cubemapHandle = loadSkyboxCubemap();
    scene.skyboxData.shaderID = createShaderProgram("skybox.vs", "skybox.fs");
    scene.skyboxData.meshVAO = scene.assetManager.meshes[2].vao; // TODO: CHANGE
    scene.sceneUBO = createSceneUBO();
    std::memset(scene.keyStateBuffer, 0, sizeof(scene.keyStateBuffer));
    std::memset(scene.lastKeyStateBuffer, 0, sizeof(scene.lastKeyStateBuffer));
}

void initScene(ECS& scene) {

    scene.meshSet.add(scene.entityCount, scene.assetManager.meshes[1]);
    scene.transformSet.add(scene.entityCount, TransformComponent{.position = glm::vec3(0.0f, 15.0f, 10.0f)});

    CameraComponent camera{
        .positionOffset = glm::vec3(0.0f),
        .position = glm::vec3(0.0f, 20.0f, 10.0f),
        .worldUp = glm::vec3(0, 1, 0),
        .yaw = -90.0f,
        .pitch = -60.0f,
        .cameraType = CameraType::FIXED};

    updateCameraVectors(camera);
    scene.cameraSet.add(scene.entityCount, camera);

    ++scene.entityCount;
    scene.meshSet.add(scene.entityCount, scene.assetManager.meshes[0]);
    scene.materialSet.add(scene.entityCount, scene.assetManager.materials[1]);
    scene.renderableSet.add(scene.entityCount, RenderableTag{});
    scene.transformSet.add(scene.entityCount, TransformComponent{.position = glm::vec3(-50, 0, 50)});

    const uint32_t CUBE = createUnitCubePrimitive(scene.assetManager.meshes).handle;
    const uint32_t MAT = 0;

    uint32_t p1 = createCube(scene, {-5.0f, 0.5f, -2.0f}, {1, 1, 1}, CUBE, MAT);
    scene.speedSet.add(p1, {5.0f});
    scene.rotationSpeedSet.add(p1, {100.0f});
    scene.inputTankSet.add(p1, PlayerInputTankTag{});
    scene.inputMapSet.add(p1, {.forwardIndex = GLFW_KEY_W - 31, .backIndex = GLFW_KEY_S - 31, 
                               .leftIndex = GLFW_KEY_A - 31, .rightIndex = GLFW_KEY_D - 31, .shootIndex = GLFW_KEY_SPACE - 31});
    scene.dynamicSet.add(p1, DynamicTag{});
    scene.healthSet.add(p1, {3});

    uint32_t p2 = createCube(scene, {5.0f, 0.5f, -2.0f}, {1, 1, 1}, CUBE, MAT);
    scene.speedSet.add(p2, {5.0f});
    scene.rotationSpeedSet.add(p2, {100.0f});
    scene.inputTankSet.add(p2, PlayerInputTankTag{});
    scene.inputMapSet.add(p2, {.forwardIndex = GLFW_KEY_I - 31, .backIndex = GLFW_KEY_K - 31, 
                               .leftIndex = GLFW_KEY_J - 31, .rightIndex = GLFW_KEY_L - 31, .shootIndex = GLFW_KEY_M - 31});
    scene.dynamicSet.add(p2, DynamicTag{});
    scene.healthSet.add(p2, {3});

    uint32_t l1 = createCube(scene, {0.0f, 0.5f, -4.0f}, {1, 1, 1}, CUBE, MAT);
    scene.pointLightSet.add(l1, {.colour = {1.0f, 1.0f, 1.0f}, .intensity = 1.0f, .radius = 10.0f});

    uint32_t l2 = createCube(scene, {-3.0f, 0.5f, 0.0f}, {1, 1, 1}, CUBE, MAT);
    scene.pointLightSet.add(l2, {.colour = {1.0f, 1.0f, 1.0f}, .intensity = 1.0f, .radius = 10.0f});

    createCube(scene, {0.0f, 0.5f, -10.0f}, {20.0f, 1.0f, 1.0f}, CUBE, MAT);
    createCube(scene, {0.0f, 0.5f, 2.0f}, {20.0f, 1.0f, 1.0f}, CUBE, MAT);
    createCube(scene, {-10.0f, 0.5f, -4.0f}, {1.0f, 1.0f, 12.0f}, CUBE, MAT);
    createCube(scene, {10.0f, 0.5f, -4.0f}, {1.0f, 1.0f, 12.0f}, CUBE, MAT);
}

GLuint createSceneUBO() {
    GLuint sceneUBO;
    glGenBuffers(1, &sceneUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, sceneUBO);

    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneUBOData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, sceneUBO);

    return sceneUBO;
}

// TODO: SKYBOX DOESNT HAVE TO BE UPDATED EVERY FRAME ONLY WHEN IT CHANGES, NOT A PRIORITY RIGHT NOW
void updateSceneData(SceneUBOData& sceneData, const CameraComponent& camera, 
                    const std::vector<PackedLightData>& visiblePointLights, const SkyboxData& skyboxData) {
    sceneData.viewMatrix = camera.viewMatrix;
    sceneData.projectionMatrix = camera.projectionMatrix;
    sceneData.cameraPosition = camera.position;
    sceneData.pointLightCount = static_cast<uint32_t>(visiblePointLights.size());
    sceneData.skyboxCubemapHandle = skyboxData.cubemapHandle;
};

void uploadSceneUBO(const GLuint sceneUBO, const SceneUBOData sceneData) {
    glBindBuffer(GL_UNIFORM_BUFFER, sceneUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneUBOData), &sceneData);
}

GLuint createLightSSBO() {
    GLuint lightSSBO;
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);

    // TODO: CHANGE WHERE THIS BELONGS - NEEDS TO BE A MODIFIABLE.
    uint32_t maxLights = 256;
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxLights * sizeof(PackedLightData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

    return lightSSBO;
}

void performLightCulling(const SparseSet<PointLightComponent>& pointLightEntities,
                         const SparseSet<TransformComponent>& transformSet,
                         std::vector<PackedLightData>& visiblePointLights,
                         const glm::vec4* frustumPlanes) {

    visiblePointLights.clear();

    for (uint32_t entity : pointLightEntities.getEntities()) {
        const auto& light = pointLightEntities.getComponent(entity);
        const auto& transform = transformSet.getComponent(entity);

        bool isInside = true;

        for (int i = 0; i < 6; ++i) {
            float distance = glm::dot(glm::vec3(frustumPlanes[i]), transform.position) + frustumPlanes[i].w;

            if (distance < -light.radius) {
                isInside = false;
                break;
            }
        }

        if (isInside) {
            visiblePointLights.emplace_back(glm::vec4(light.colour, light.intensity), glm::vec4(transform.position, light.radius));
        }
    }
}

void uploadLightSSBO(const GLuint lightSSBO, const std::vector<PackedLightData>& visiblePointLights) {
    if (visiblePointLights.size() == 0) return;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, visiblePointLights.size() * sizeof(PackedLightData), visiblePointLights.data());
}

void performFrustumCulling(const std::vector<uint32_t>& renderableEntities, // TODO: KIND OF WEIRD
                           const SparseSet<TransformComponent>& transformSet,
                           const SparseSet<MeshData>& meshSet,
                           std::vector<uint32_t>& visibleEntities,
                           const glm::vec4* frustumPlanes) {

    visibleEntities.clear();
    for (uint32_t entity : renderableEntities) {
        const TransformComponent& transform = transformSet.getComponent(entity);
        const MeshData& mesh = meshSet.getComponent(entity);

        // ARVO METHOD
        // TODO: EXPLAIN ARVO METHOD
        glm::mat3 R = glm::mat3_cast(transform.rotation);

        glm::vec3 localCenter = glm::vec3(
            (mesh.localAABB.minX + mesh.localAABB.maxX) * 0.5f,
            (mesh.localAABB.minY + mesh.localAABB.maxY) * 0.5f,
            (mesh.localAABB.minZ + mesh.localAABB.maxZ) * 0.5f);
        glm::vec3 localExtent = glm::vec3(
            (mesh.localAABB.maxX - mesh.localAABB.minX) * 0.5f,
            (mesh.localAABB.maxY - mesh.localAABB.minY) * 0.5f,
            (mesh.localAABB.maxZ - mesh.localAABB.minZ) * 0.5f);

        glm::vec3 worldCenter = transform.position + (R * (localCenter * transform.scale));

        glm::vec3 worldExtent;
        for (int i = 0; i < 3; ++i) {
            worldExtent[i] =
                glm::abs(R[0][i] * transform.scale.x) * localExtent.x +
                glm::abs(R[1][i] * transform.scale.y) * localExtent.y +
                glm::abs(R[2][i] * transform.scale.z) * localExtent.z;
        }

        bool isInside = true;
        // A frustum is always made up of six planes.
        for (int i = 0; i < 6; ++i) {
            const glm::vec4& plane = frustumPlanes[i];

            float r = worldExtent.x * glm::abs(plane.x) +
                      worldExtent.y * glm::abs(plane.y) +
                      worldExtent.z * glm::abs(plane.z);

            float s = glm::dot(glm::vec3(plane), worldCenter) + plane.w;

            if (s < -r) {
                isInside = false;
                break;
            }
        }

        if (isInside) {
            visibleEntities.push_back(entity);
        }
    }
}

Framebuffer createFrameBuffer(const uint32_t framebufferShaderID, const uint32_t width, const uint32_t height) {
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                  << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return Framebuffer{.buffer = framebuffer,
                       .textureAttachment = textureColorbuffer,
                       .renderBufferObject = rbo,
                       .shaderID = framebufferShaderID};
}

GLuint createQuad() {
    float quadVertices[] = {-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f,
                            0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f,
                            1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    return quadVAO;
}

bool eventQueueEmpty(EventQueue& queue) {
    return queue.front == queue.back;
}

// There is one index always left empty between the back and the front to distinguish between an empty queue and a full queue.
void pushEvent(EventQueue& queue, const Event& event) {
    uint8_t next = (queue.back + 1) & 127;
    if (next != queue.front) {
        queue.ringBuffer[queue.back] = event;
        queue.back = next;
    }
}

Event popEvent(EventQueue& queue) {
    uint8_t front = queue.front;
    queue.front = (queue.front + 1) & 127;
    return queue.ringBuffer[front];
}

bool pollEvent(EventQueue& eventQueue, Event& event) {
    if (!eventQueueEmpty(eventQueue)) {
        event = popEvent(eventQueue);
        return true;
    }
    return false;
}

void handleWindowEvent(const Event& event, WindowData& window, Framebuffer& framebuffer, CameraComponent& camera,
                       MouseData& mouseData) {

    switch (event.type) {
    case EventType::WindowResize: {
        glViewport(0, 0, event.resize.width, event.resize.height);
        window.width = event.resize.width;
        window.height = event.resize.height;
        framebuffer = createFrameBuffer(framebuffer.shaderID, window.width, window.height);
        updateProjectionMatrix(camera, window.width, window.height);
        break;
    }

    case EventType::MouseMove: {
        if (!mouseData.hasBeenRecorded) {
            mouseData.lastCursorX = event.mouseMove.xPos;
            mouseData.lastCursorY = event.mouseMove.yPos;
            mouseData.hasBeenRecorded = true;
        } 
        mouseData.frameOffsetX += event.mouseMove.xPos - mouseData.lastCursorX;
        mouseData.frameOffsetY += mouseData.lastCursorY - event.mouseMove.yPos;

        mouseData.lastCursorX = event.mouseMove.xPos;
        mouseData.lastCursorY = event.mouseMove.yPos;

        break;
    }
    case EventType::Scroll: {
        processMouseScroll(camera, event.scroll.yOffset);
        break;
    }
    }
}

void bulletSystem(ECS& scene) {
    for (uint32_t entity : scene.inputMapSet.getEntities()) {
        const TransformComponent& transform = scene.transformSet.getComponent(entity);
        const InputMapComponent& inputMap = scene.inputMapSet.getComponent(entity);
        if (scene.keyStateBuffer[inputMap.shootIndex] && !scene.lastKeyStateBuffer[inputMap.shootIndex]) {
            createBullet(scene, transform.position, transform.rotation);
            scene.keyStateBuffer[inputMap.shootIndex] = 0.0f;
        }
    }
}

void createBullet(ECS& scene, glm::vec3 position, glm::quat rotation) {
    ++scene.entityCount;
    scene.meshSet.add(scene.entityCount, createUnitCubePrimitive(scene.assetManager.meshes));
    scene.materialSet.add(scene.entityCount, scene.assetManager.materials[0]);
    scene.renderableSet.add(scene.entityCount, RenderableTag{});
    glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    float bulletOffset = 1.0f;
    position += bulletOffset * front;
    scene.transformSet.add(scene.entityCount, TransformComponent{.position = position, .scale = glm::vec3(0.2f)});
    scene.collisionSet.add(scene.entityCount, CollisionComponent{.minX = -0.1f, .maxX = 0.1f, .minY = -0.1f, .maxY = 0.1f, .minZ = -0.1f, .maxZ = 0.1f});
    scene.velocitySet.add(scene.entityCount, VelocityComponent{glm::vec3(10.0f * front)});
    scene.bulletSet.add(scene.entityCount, BulletTag{});
    scene.dynamicSet.add(scene.entityCount, DynamicTag{});
}


/*
* This May be necessary for users.
struct Entity {
    uint32_t id;
    ECS* scene;

    void move(glm::vec3 offset) {
        auto& transform = scene->transformSet.getComponent(id);
        transform.position += offset;
    }

    bool hasInput(int keyIndex) {
        return scene->keyStateBuffer[keyIndex] > 0.5f;
    }
};
*/