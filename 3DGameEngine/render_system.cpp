#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "shader_s.h"
#include <unordered_map>
#include "camera.h"
#include "window.h"
#include "asset_manager.h"
#include "entity.h"
#include "ecs.h"
#include "text.hpp"
#include "render_system.h"



RenderSystem::RenderSystem(Window &window)
    : window(window),
      framebufferShader("fb_vertex_shader.vs", "fb_fragment_shader.fs"),
      framebuffer(createFrameBuffer(framebufferShader, window.width, window.height)),
      quadVAO(createQuad())
{
    initOpenglState();
}

void RenderSystem::renderScene(ECS& scene) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CameraComponent& camera = scene.cameraSet.getComponent(scene.cameraSet.getEntities()[0]);
    for (uint32_t entity : scene.renderableSet.getEntities()) {
        MaterialData& material = scene.materialSet.getComponent(entity);
        MeshData& mesh = scene.meshSet.getComponent(entity);
        TransformComponent& transform = scene.transformSet.getComponent(entity);
        material.shader.use();
        material.shader.setMat4Uniform("projection", camera.projectionMatrix);
        if (scene.skyboxSet.hasComponent(entity)) {
            glm::mat4 skyboxViewMatrix(glm::mat3(camera.viewMatrix));
            material.shader.setMat4Uniform("view", skyboxViewMatrix);
            glDepthFunc(GL_LEQUAL);
        }
        else {
            material.shader.setMat4Uniform("view", camera.viewMatrix);
            material.shader.setMat4Uniform("model", transform.transform);
            material.shader.setVec3Uniform("cameraPos", camera.position);
        }
        glBindVertexArray(mesh.vao);
        for (int i = 0; i < material.textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(material.textures[i].target, material.textures[i].id);
        }
        if (!scene.instancedSet.hasComponent(entity)) {
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        }
        else {
            glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.vertexCount, scene.instancedSet.getComponent(entity).numberOfInstances);
        }
        glDepthFunc(GL_LESS);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    framebuffer.shader.use();
    glBindVertexArray(quadVAO);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer.textureAttachment);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}


void RenderSystem::initOpenglState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
}

GLuint RenderSystem::createQuad() {
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };
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



Framebuffer createFrameBuffer(Shader& framebufferShader, uint32_t width, uint32_t height) {
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
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return Framebuffer{
        .buffer = framebuffer,
        .textureAttachment = textureColorbuffer,
        .renderBufferObject = rbo,
        .shader = framebufferShader
    };
}