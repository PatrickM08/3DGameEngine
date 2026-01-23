#pragma once
#include <glad/glad.h>
#include <cstdint>
#include "shader_s.h"
#include "window.h"
#include "ecs.h"

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
};

// TODO: THE CONST-CORRECTNESS HERE IS UNNECSSARY - NOT SURE IF I LIKE IT
Framebuffer createFrameBuffer(const uint32_t frambufferShaderID, const uint32_t width, const uint32_t height);
glm::mat4 buildTransformMatrix(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation);
void performFrustumCulling(const std::vector<uint32_t>& renderableEntities,
                           const SparseSet<TransformComponent>& transformSet,
                           const SparseSet<MeshData>& meshSet,
                           const SparseSet<SkyboxTag>& skyboxSet,
                           std::vector<uint32_t>& visibleEntities, 
                           const glm::vec4* frustumPlanes);

void performLightCulling(const SparseSet<PointLightComponent>& pointLightEntities,
                         const SparseSet<TransformComponent>& transformSet,
                         std::vector<PackedLightData>& visiblePointLights,
                         const glm::vec4* frustumPlanes);

void uploadLightSSBO(const GLuint lightSSBO, const std::vector<PackedLightData>& lightData);
void uploadSceneUBO(const GLuint sceneUBO, const SceneUBOData sceneData);

// TODO: NEED TO SEE WHETHER THIS STATE IS ACCEPTABLE FOR DLL
class RenderSystem {
public:
    RenderSystem(Window& window);
    void renderScene(ECS& scene);
    uint32_t framebufferShaderID;
    Framebuffer framebuffer;
    
private:
    void initOpenglState();
    GLuint createQuad();
    GLuint createSceneUBO();
    GLuint createLightSSBO();

    Window window;
    GLuint quadVAO;
    GLuint sceneUBO;
    GLuint lightSSBO;
};