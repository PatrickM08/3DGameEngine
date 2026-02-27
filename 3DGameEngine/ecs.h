#pragma once
#include "asset_manager.h"
#include "camera.h"
#include "entity.h"
#include "sparse_set.h"
#include <cstdint>
#include <cstring>
#include <glm/glm.hpp>

// TODO: ADD DEFAULTS

struct WindowData {
    uint32_t width;
    uint32_t height;
    const char* title; // TODO: THIS MIGHT BE BETTER AS A BUFFER CHECK LATER.
    GLFWwindow* windowPtr;
};

struct Framebuffer {
    GLuint buffer;
    GLuint textureAttachment;
    GLuint renderBufferObject;
    uint32_t width, height;
    uint32_t shaderID;
};

struct SceneUBOData {
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 cameraPosition;
    uint32_t pointLightCount;
    uint64_t skyboxCubemapHandle;
};

struct SkyboxData {
    uint64_t cubemapHandle;
    uint32_t shaderID;
    uint32_t meshVAO;
};

struct PhysicsManifoldEntry {
    uint32_t entityID;
    float depth;
    glm::vec3 collisionNormal;
};

// TODO: The capacity and size of these can probably be 16 bit - but we can leave it for now
struct CollisionPhysicsManifold {
    static constexpr uint32_t capacity = 64;
    uint32_t size = 0;
    PhysicsManifoldEntry buffer[capacity];
};

struct DeleteBuffer {
    static constexpr uint32_t capacity = 64;
    uint32_t size = 0;
    uint32_t buffer[capacity];
};

enum class EventType {
    WindowResize,
    MouseMove,
    Scroll
};

struct Event {
    EventType type;
    union {
        struct {
            int width, height;
        } resize;
        struct {
            float xPos, yPos;
        } mouseMove;
        struct {
            float yOffset;
        } scroll;
    };
};

// TODO: NEED TO FIND HOW MUCH SPACE I ACTUALLY NEED FOR THIS.

struct EventQueue {
    Event ringBuffer[128];
    uint8_t front = 0;
    uint8_t back = 0;
};

bool eventQueueEmpty(EventQueue& queue);

// There is one index always left empty between the back and the front to distinguish between an empty queue and a full queue.
void pushEvent(EventQueue& queue, const Event& event);

Event popEvent(EventQueue& queue);

bool pollEvent(EventQueue& eventQueue, Event& event);

struct MouseData {
    float lastCursorX = 0.0f;
    float lastCursorY = 0.0f;
    float frameOffsetX = 0.0f;
    float frameOffsetY = 0.0f;
    bool hasBeenRecorded = false;
};

GLuint createLightSSBO();
void performLightCulling(const SparseSet<PointLightComponent>& pointLightEntities,
                         const SparseSet<TransformComponent>& transformSet,
                         std::vector<PackedLightData>& visiblePointLights,
                         const glm::vec4* frustumPlanes);
void uploadLightSSBO(const GLuint lightSSBO, const std::vector<PackedLightData>& visiblePointLights);

GLuint createSceneUBO();
void updateSceneData(SceneUBOData& sceneData, const CameraComponent& camera, 
                    const std::vector<PackedLightData>& visiblePointLights, const SkyboxData& skyboxData);
void uploadSceneUBO(const GLuint sceneUBO, const SceneUBOData sceneData);

void performFrustumCulling(const std::vector<uint32_t>& renderableEntities,
                           const SparseSet<TransformComponent>& transformSet,
                           const SparseSet<MeshData>& meshSet,
                           std::vector<uint32_t>& visibleEntities,
                           const glm::vec4* frustumPlanes);

Framebuffer createFrameBuffer(const uint32_t frambufferShaderID, const uint32_t width, const uint32_t height);
GLuint createQuad();

struct ECS {
    // Currently there are no paged sparse sets as there are no sparse
    // components that are spaced far apart. i.e. greater than one entity has a
    // component and these entities differ largely in entity count.
    // 
    // TODO: NEED TO RESERVE MEMORY ONCE WE GET DYNAMIC ENTITY CREATION - THIS SHOULD BE AN ARENA FOR ALL COMPONENTS
    // TODO: SPARSE SETS SHOULD BE MADE CLEANER
    SparseSet<TransformComponent> transformSet;
    SparseSet<MeshData> meshSet;
    SparseSet<MaterialData> materialSet;
    SparseSet<SkyboxTag> skyboxSet;
    SparseSet<InstancedComponent> instancedSet;
    SparseSet<PlayerInputWorldTag> inputWorldSet;
    SparseSet<PlayerInputTankTag> inputTankSet;
    SparseSet<PlayerInputNoClipTag> inputNoClipSet;
    SparseSet<VelocityComponent> velocitySet;
    SparseSet<SpeedComponent> speedSet;
    SparseSet<RotationSpeedComponent> rotationSpeedSet;
    SparseSet<PatrolComponent> patrolSet;
    SparseSet<CollisionComponent> collisionSet;
    SparseSet<RenderableTag> renderableSet;
    SparseSet<CameraComponent> cameraSet;
    SparseSet<PointLightComponent> pointLightSet;
    SparseSet<BulletTag> bulletSet;
    SparseSet<DynamicTag> dynamicSet;
    SparseSet<HealthComponent> healthSet;
    SparseSet<InputMapComponent> inputMapSet;

    WindowData window;
    AssetManager assetManager;

    uint32_t entityCount;

    Framebuffer framebuffer;
    GLuint quadVAO;
    
    std::vector<uint32_t> visibleEntities;
    std::vector<PackedLightData> visiblePointLights;
    GLuint lightSSBO;
    SkyboxData skyboxData;
    GLuint sceneUBO;
    SceneUBOData sceneData;

    CollisionPhysicsManifold physicsManifold;
    DeleteBuffer deleteBuffer;
    // TODO: so the idea is that allowing 0 as null will make it easier to have a null mapping, but I need to check performance compared to tags.
    float keyStateBuffer[318];
    float lastKeyStateBuffer[318];
    EventQueue eventQueue;
    MouseData mouseData;
};

void initState(ECS& scene);

void initScene(ECS& scene);

void handleWindowEvent(const Event& event, WindowData& window, Framebuffer& framebuffer, CameraComponent& camera,
                       MouseData& mouseData);

void createBullet(ECS& scene, glm::vec3 position, glm::quat rotation);

void bulletSystem(ECS& scene);