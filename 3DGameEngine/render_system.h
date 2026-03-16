#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdint>
#include "sparse_set.h"
#include "entity.h"

struct MaterialData;
struct MeshData;
struct CameraComponent;

struct VisibleEntityBuffer {
    uint32_t size = 0;
    uint32_t buffer[MAX_ENTITIES];
};

struct VisiblePointLightBuffer {
    static constexpr uint16_t capacity = 256;
    uint16_t size = 0;
    PackedLightData buffer[capacity];
};

struct Framebuffer {
    GLuint buffer;
    GLuint textureAttachment;
    GLuint renderBufferObject;
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

// TODO: THE CONST-CORRECTNESS HERE IS UNNECSSARY - NOT SURE IF I LIKE IT
glm::mat4 buildTransformMatrix(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation);
void renderSkybox(const SkyboxData& skyboxData);
void renderSystem(const VisibleEntityBuffer& visibleEntities, const SparseSet<MaterialData>& materialSet,
                 const SparseSet<MeshData>& meshSet, const SparseSet<TransformComponent>& transformSet, const Framebuffer& framebuffer);
void drawToFramebuffer(const Framebuffer& framebuffer, uint32_t quadVAO);
void initOpenglRenderState();
uint32_t createLightSSBO(uint32_t maxLights);
void performLightCulling(const SparseSet<PointLightComponent>& pointLightEntities,
                         const SparseSet<TransformComponent>& transformSet,
                         VisiblePointLightBuffer& visiblePointLights,
                         const glm::vec4* frustumPlanes);
void uploadLightSSBO(const uint32_t lightSSBO, const VisiblePointLightBuffer& visiblePointLights);

uint32_t createSceneUBO();
void updateSceneData(SceneUBOData& sceneData, const CameraComponent& camera,
                     const VisiblePointLightBuffer& visiblePointLights, const SkyboxData& skyboxData);
void uploadSceneUBO(const uint32_t sceneUBO, const SceneUBOData sceneData);

void performFrustumCulling(const SparseSet<RenderableTag>& renderableEntities,
                           const SparseSet<TransformComponent>& transformSet,
                           const SparseSet<MeshData>& meshSet,
                           VisibleEntityBuffer& visibleEntities,
                           const glm::vec4* frustumPlanes);

Framebuffer createFrameBuffer(const uint32_t frambufferShaderID, const uint32_t width, const uint32_t height);
uint32_t createQuad();