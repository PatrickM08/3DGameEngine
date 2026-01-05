#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include "shader_s.h"
#include "camera.h"
#include "window.h"
#include "asset_manager.h"
#include "entity.h"
#include "ecs.h"
#include "render_system.h"

RenderSystem::RenderSystem(Window& window)
    : window(window),
      framebufferShader("fb_vertex_shader.vs", "fb_fragment_shader.fs"),
      framebuffer(createFrameBuffer(framebufferShader, window.width, window.height)),
      quadVAO(createQuad()) {
    initOpenglState();
}

void RenderSystem::renderScene(ECS& scene) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CameraComponent& camera = scene.cameraSet.getComponent(scene.cameraSet.getEntities()[0]);
    std::vector<uint32_t>& visibleEntities = scene.visibleEntities;
    // TODO: DONT FORGET TO REMOVE THIS - and look into iostream usage throughout, including exceptions.
    static std::vector<uint32_t> prevVE;
    // TODO: MAKE FRUSTUM CULLLING OPTIONAL
    performFrustumCulling(scene.renderableSet.getEntities(), scene.transformSet, scene.meshSet, scene.skyboxSet, visibleEntities, camera.frustumPlanes);
    if (prevVE != visibleEntities) {
        std::cout << "Visible entities changed! Count: " << visibleEntities.size() << "\n";
        for (auto i : visibleEntities) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
    prevVE = visibleEntities;
    for (uint32_t entity : visibleEntities) {
        MaterialData& material = scene.materialSet.getComponent(entity);
        MeshData& mesh = scene.meshSet.getComponent(entity);
        TransformComponent& transform = scene.transformSet.getComponent(entity);
        glm::mat4 transformMatrix = buildTransformMatrix(transform.position, transform.scale, transform.rotation);
        material.shader.use();
        // TODO: This needs to be looked at - it would be more efficient to use a cached VP matrix but I don't know if that would cause issues later.
        material.shader.setMat4Uniform("projection", camera.projectionMatrix);
        if (scene.skyboxSet.hasComponent(entity)) {
            glm::mat4 skyboxViewMatrix(glm::mat3(camera.viewMatrix));
            material.shader.setMat4Uniform("view", skyboxViewMatrix);
            glDepthFunc(GL_LEQUAL);
        } else {
            material.shader.setMat4Uniform("view", camera.viewMatrix);
            material.shader.setMat4Uniform("model", transformMatrix);
            material.shader.setVec3Uniform("cameraPos", camera.position);
        }
        glBindVertexArray(mesh.vao);
        for (int i = 0; i < material.textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(material.textures[i].target, material.textures[i].id);
        }
        if (!scene.instancedSet.hasComponent(entity)) {
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        } else {
            glDrawArraysInstanced(
                GL_TRIANGLES, 0, mesh.vertexCount,
                scene.instancedSet.getComponent(entity).numberOfInstances);
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

Framebuffer createFrameBuffer(const Shader& framebufferShader, const uint32_t width, const uint32_t height) {
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
                       .shader = framebufferShader};
}

glm::mat4 buildTransformMatrix(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation) {
    glm::mat3 rotationMatrix = glm::mat3_cast(rotation);

    glm::mat4 transformMatrix;
    transformMatrix[0] = glm::vec4(rotationMatrix[0] * scale.x, 0.0f);
    transformMatrix[1] = glm::vec4(rotationMatrix[1] * scale.y, 0.0f);
    transformMatrix[2] = glm::vec4(rotationMatrix[2] * scale.z, 0.0f);
    transformMatrix[3] = glm::vec4(position, 1.0f);

    return transformMatrix;
}

void performFrustumCulling(const std::vector<uint32_t>& renderableEntities,
                           const SparseSet<TransformComponent>& transformSet, // All renderable entities will have a transform.
                           const SparseSet<MeshData>& meshSet,
                           const SparseSet<SkyboxTag>& skyboxSet,
                           std::vector<uint32_t>& visibleEntities,
                           const glm::vec4* frustumPlanes) {

    visibleEntities.clear();
    for (uint32_t entity : renderableEntities) {
        // Skybox is never culled - tricky to make it work with the skybox.
        if (skyboxSet.hasComponent(entity)) {
            visibleEntities.push_back(entity);
            continue;
        }
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