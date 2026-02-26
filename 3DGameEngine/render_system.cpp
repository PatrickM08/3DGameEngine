#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "entity.h"
#include "ecs.h"
#include "render_system.h"

void renderSystem(const std::vector<uint32_t>& visibleEntities, const SparseSet<MaterialData>& materialSet, 
                  const SparseSet<MeshData>& meshSet, const SparseSet<TransformComponent>& transformSet, const Framebuffer& framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (uint32_t entity : visibleEntities) {
        const MaterialData& material = materialSet.getComponent(entity);
        const MeshData& mesh = meshSet.getComponent(entity);
        // TODO: This needs to be looked at - it would be more efficient to use a cached VP matrix but I don't know if that would cause issues later.
        const TransformComponent& transform = transformSet.getComponent(entity);
        // TODO: I THINK THIS IS WRONG NOW, NEED TO PROFILE
        glm::mat4 transformMatrix = buildTransformMatrix(transform.position, transform.scale, transform.rotation);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transformMatrix)));
        glProgramUniformMatrix4fv(material.shaderID, 0, 1, GL_FALSE, &transformMatrix[0][0]);
        glProgramUniformMatrix3fv(material.shaderID, 1, 1, GL_FALSE, &normalMatrix[0][0]);

        glBindVertexArray(mesh.vao);
        glProgramUniform1i(material.shaderID, 2, material.materialSSBOIndex);
        glUseProgram(material.shaderID);
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        glDepthFunc(GL_LESS);
    }
}

// TODO: MAKE AN INSTANCE SYSTEM, NOT A PRIORITY

void drawToFramebuffer(const Framebuffer& framebuffer, GLuint quadVAO) {
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
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
};

void initOpenglRenderState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
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