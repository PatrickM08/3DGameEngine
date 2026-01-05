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
    Shader shader;
};

Framebuffer createFrameBuffer(const Shader& framebufferShader, const uint32_t width, const uint32_t height);
glm::mat4 buildTransformMatrix(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation);
void performFrustumCulling(const std::vector<uint32_t>& renderableEntities,
                           const SparseSet<TransformComponent>& transformSet,
                           const SparseSet<MeshData>& meshSet,
                           const SparseSet<SkyboxTag>& skyboxSet,
                           std::vector<uint32_t>& visibleEntities, 
                           const glm::vec4* frustumPlanes);

class RenderSystem {
public:
    RenderSystem(Window& window);
    void renderScene(ECS& scene);
    Shader framebufferShader;
    Framebuffer framebuffer;
    

private:
    void initOpenglState();
    GLuint createQuad();

    Window window;
    GLuint quadVAO;
    std::vector<uint32_t> renderQueue;
};