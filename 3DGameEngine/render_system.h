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

Framebuffer createFrameBuffer(const char* vsPath, const char* fsPath, uint32_t width, uint32_t height);

class RenderSystem {
public:
    RenderSystem(Window& window);
    void renderScene(ECS& scene);

private:
    void initOpenglState();
    GLuint createQuad();

    Window window;
    Framebuffer framebuffer;
    GLuint quadVAO;
};