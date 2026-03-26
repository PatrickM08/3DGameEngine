#include "render_system.h"
#include "asset_manager.h"
#include "entity.h"
#include "camera.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void renderSystem(const VisibleEntityBuffer& visibleEntityBuffer, const SparseSet<MaterialData>& materialSet, 
                  const SparseSet<MeshData>& meshSet, const SparseSet<TransformComponent>& transformSet, const Framebuffer& framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (uint32_t i = 0; i < visibleEntityBuffer.size; ++i) {
        uint32_t entity = visibleEntityBuffer.buffer[i];
        const MaterialData& material = materialSet.getComponent(entity);
        const MeshData& mesh = meshSet.getComponent(entity);
        const TransformComponent& transform = transformSet.getComponent(entity);
        glm::mat4 transformMatrix = buildTransformMatrix(transform.position, transform.scale, transform.rotation);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transformMatrix)));
        glProgramUniformMatrix4fv(material.shaderID, 0, 1, GL_FALSE, &transformMatrix[0][0]);
        glProgramUniformMatrix3fv(material.shaderID, 1, 1, GL_FALSE, &normalMatrix[0][0]);

        glBindVertexArray(mesh.vao);
        glProgramUniform1i(material.shaderID, 2, material.materialSSBOIndex);
        glUseProgram(material.shaderID);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
    }
}

void drawToFramebuffer(const Framebuffer& framebuffer, uint32_t quadVAO) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(framebuffer.shaderID);
    glBindVertexArray(quadVAO);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer.textureAttachment);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
};

void renderSkybox(const SkyboxData& skyboxData) {
    glBindVertexArray(skyboxData.meshVAO);
    glUseProgram(skyboxData.shaderID);
    glDepthFunc(GL_LEQUAL);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glDepthFunc(GL_LESS);
};

glm::mat4 buildTransformMatrix(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation) {
    glm::mat3 rotationMatrix = glm::mat3_cast(rotation);

    glm::mat4 transformMatrix;
    transformMatrix[0] = glm::vec4(rotationMatrix[0] * scale.x, 0.0f);
    transformMatrix[1] = glm::vec4(rotationMatrix[1] * scale.y, 0.0f);
    transformMatrix[2] = glm::vec4(rotationMatrix[2] * scale.z, 0.0f);
    transformMatrix[3] = glm::vec4(position, 1.0f);

    return transformMatrix;
}

void performLightCulling(const SparseSet<PointLightComponent>& pointLightSet,
                         const SparseSet<TransformComponent>& transformSet,
                         VisiblePointLightBuffer& visiblePointLightBuffer,
                         const glm::vec4* frustumPlanes) {

    visiblePointLightBuffer.size = 0;

    for (uint32_t i = 0; i < pointLightSet.entityCount; ++i) {
        uint32_t entity = pointLightSet.entities[i];

        const auto& light = pointLightSet.getComponent(entity);
        const auto& transform = transformSet.getComponent(entity);

        bool isInside = true;

        for (int i = 0; i < 6; ++i) {
            float distance = glm::dot(glm::vec3(frustumPlanes[i]), transform.position) + frustumPlanes[i].w;

            if (distance < -light.radius) {
                isInside = false;
                break;
            }
        }

        if (isInside) {
            // TODO: ADD SAFETY CHECK
            visiblePointLightBuffer.buffer[visiblePointLightBuffer.size++] = PackedLightData{glm::vec4(light.colour, light.intensity),
                                                                                             glm::vec4(transform.position, light.radius)};
        }
    }
}

void performFrustumCulling(const SparseSet<RenderableTag>& renderableSet,
                           const SparseSet<TransformComponent>& transformSet,
                           const SparseSet<MeshData>& meshSet,
                           VisibleEntityBuffer& visibleEntityBuffer,
                           const glm::vec4* frustumPlanes) {

    visibleEntityBuffer.size = 0;
    for (uint32_t i = 0; i < renderableSet.entityCount; ++i) {
        uint32_t entity = renderableSet.entities[i];
        const TransformComponent& transform = transformSet.getComponent(entity);
        const MeshData& mesh = meshSet.getComponent(entity);

        // ARVO METHOD
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
            visibleEntityBuffer.buffer[visibleEntityBuffer.size++] = entity;
        }
    }
}

GLuint createSceneUBO() {
    GLuint sceneUBO;
    glGenBuffers(1, &sceneUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, sceneUBO);

    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneUBOData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, sceneUBO);

    return sceneUBO;
}

void updateSceneData(SceneUBOData& sceneData, const CameraComponent& camera,
                     const VisiblePointLightBuffer& visiblePointLightBuffer, const SkyboxData& skyboxData) {
    sceneData.viewMatrix = camera.viewMatrix;
    sceneData.projectionMatrix = camera.projectionMatrix;
    sceneData.cameraPosition = camera.position;
    sceneData.pointLightCount = (uint32_t)visiblePointLightBuffer.size;
    sceneData.skyboxCubemapHandle = skyboxData.cubemapHandle;
};

void uploadSceneUBO(const uint32_t sceneUBO, const SceneUBOData sceneData) {
    glBindBuffer(GL_UNIFORM_BUFFER, sceneUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneUBOData), &sceneData);
}

uint32_t createLightSSBO(uint32_t maxLights) {
    uint32_t lightSSBO;
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);

    glBufferData(GL_SHADER_STORAGE_BUFFER, maxLights * sizeof(PackedLightData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

    return lightSSBO;
}

void uploadLightSSBO(const uint32_t lightSSBO, const VisiblePointLightBuffer& visiblePointLightBuffer) {
    if (visiblePointLightBuffer.size == 0) return;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);

    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                    visiblePointLightBuffer.size * sizeof(PackedLightData),
                    visiblePointLightBuffer.buffer);
}

Framebuffer createFrameBuffer(const uint32_t framebufferShaderID, const uint32_t width, const uint32_t height) {
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
                       .shaderID = framebufferShaderID};
}

uint32_t createQuad() {
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

void initOpenglRenderState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
}