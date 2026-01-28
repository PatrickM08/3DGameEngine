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

// TODO: THE CONST-CORRECTNESS HERE IS UNNECSSARY - NOT SURE IF I LIKE IT
Framebuffer createFrameBuffer(const uint32_t frambufferShaderID, const uint32_t width, const uint32_t height);
glm::mat4 buildTransformMatrix(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation);
void renderSkybox(const SkyboxData skyboxData);


// TODO: REMOVE CLASS, STORE FRAMEBUFFER DATA ELSEWHERE
class RenderSystem {
public:
    RenderSystem(Window& window);
    void renderScene(const std::vector<uint32_t>& visibleEntities, const SparseSet<MaterialData>& materialSet,
                     const SparseSet<MeshData>& meshSet, const SparseSet<TransformComponent>& transformSet);
    void drawToFramebuffer();
    uint32_t framebufferShaderID;
    Framebuffer framebuffer;
    
private:
    void initOpenglState();
    GLuint createQuad();

    Window window;
    GLuint quadVAO;
};