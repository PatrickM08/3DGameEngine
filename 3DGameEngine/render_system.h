#pragma once
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


struct Framebuffer {
    GLuint buffer;
    GLuint textureAttachment;
    GLuint renderBufferObject;
    uint32_t width, height;
};

Framebuffer createFrameBuffer(uint32_t width, uint32_t height);


class RenderSystem {
public:
    RenderSystem(Window& window) {
        initOpenglState();
        Framebuffer framebuffer = createFrameBuffer(window.width, window.height);
    }

    void renderScene(Camera& camera, ECS& scene) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (Entity e : scene.entitiesInScene) {
            MaterialData& material = scene.materialsInScene[e.id];
            MeshData& mesh = scene.meshesInScene[e.id];
            TransformComponent& transform = scene.transformsInScene[e.id];
            material.shader.use();
            material.shader.setMat4Uniform("projection", camera.projectionMatrix);
            if (scene.skyboxesInScene[e.id].isSkybox) {
                glm::mat4 skyboxViewMatrix(glm::mat3(camera.viewMatrix));
                material.shader.setMat4Uniform("view", skyboxViewMatrix);
                glDepthFunc(GL_LEQUAL);
            }
            else {
                material.shader.setMat4Uniform("view", camera.viewMatrix);
                material.shader.setMat4Uniform("model", transform.transform);
                material.shader.setVec3Uniform("cameraPos", camera.Position);
            }
            glBindVertexArray(mesh.vao);
            for (int i = 0; i < material.textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(material.textures[i].target, material.textures[i].id);
            }
            if (mesh.handle != 1) {
                glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
            }
            else {
                glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.vertexCount, 1024);
            }
            glDepthFunc(GL_LESS);
        }
    }
private:
    void initOpenglState() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    }
};


Framebuffer createFrameBuffer(uint32_t width, uint32_t height) {
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
        .renderBufferObject = rbo
    };
}