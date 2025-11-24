#pragma once
#include <glad/glad.h>
#include <cstdint>  // for uint32_t
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

Framebuffer createFrameBuffer(Shader& framebufferShader, uint32_t width, uint32_t height);

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
};