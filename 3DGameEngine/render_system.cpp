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

glm::mat4 buildTransformMatrix(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation) {
    glm::mat3 rotationMatrix = glm::mat3_cast(rotation);

    glm::mat4 transformMatrix;
    transformMatrix[0] = glm::vec4(rotationMatrix[0] * scale.x, 0.0f);
    transformMatrix[1] = glm::vec4(rotationMatrix[1] * scale.y, 0.0f);
    transformMatrix[2] = glm::vec4(rotationMatrix[2] * scale.z, 0.0f);
    transformMatrix[3] = glm::vec4(position, 1.0f);

    return transformMatrix;
}

void renderText(const char* text, int textLength, const float xPos, const float yPos,
                const float size, const uint32_t screenWidth,
                const uint32_t screenHeight, TextRenderData& textRenderData) {
    Glyph* glyphs = textRenderData.glyphs;
    GLuint& textShaderID = textRenderData.textShaderID;
    glUseProgram(textShaderID);
    glBindVertexArray(textRenderData.textVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textRenderData.bitmapFontTextureID);

    glUniform2f(0, xPos, yPos);                // 0 = screenPosition location
    glUniform1f(1, size);                      // 1 = scale location
    glUniform2f(2, screenWidth, screenHeight); // 2 = screenSize location

    float* vertices = textRenderData.vertices;
    int elementsToUse = textLength * 24;
    float currentX = 0.0f;
    for (int i = 0; i < textLength; i++) {
        Glyph glyph = glyphs[text[i] - 32]; // TODO: THIS WONT WORK FOR BLANK CHARACTERS I THINK

        int base = i * 24;

        float left = currentX + glyph.xoffset;
        float right = left + glyph.width;
        float bottom = -glyph.yoffset;
        float top = -glyph.yoffset + glyph.height;

        // Top-left vertex
        vertices[base + 0] = left;
        vertices[base + 1] = top;
        vertices[base + 2] = glyph.u0;
        vertices[base + 3] = glyph.v0;

        // Bottom-left vertex
        vertices[base + 4] = left;
        vertices[base + 5] = bottom;
        vertices[base + 6] = glyph.u0;
        vertices[base + 7] = glyph.v1;

        // Bottom-right vertex
        vertices[base + 8] = right;
        vertices[base + 9] = bottom;
        vertices[base + 10] = glyph.u1;
        vertices[base + 11] = glyph.v1;

        // Top-left vertex
        vertices[base + 12] = left;
        vertices[base + 13] = top;
        vertices[base + 14] = glyph.u0;
        vertices[base + 15] = glyph.v0;

        // Bottom-right vertex
        vertices[base + 16] = right;
        vertices[base + 17] = bottom;
        vertices[base + 18] = glyph.u1;
        vertices[base + 19] = glyph.v1;

        // Top-right vertex
        vertices[base + 20] = right;
        vertices[base + 21] = top;
        vertices[base + 22] = glyph.u1;
        vertices[base + 23] = glyph.v0;

        currentX += glyph.xadvance;
    }
    glBindBuffer(GL_ARRAY_BUFFER, textRenderData.textVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, elementsToUse * sizeof(float),
                    vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6 * textLength);
}
