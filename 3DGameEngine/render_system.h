#pragma once
#include <glad/glad.h>
#include <cstdint>
#include "ecs.h"
#include "sparse_set.h"
#include "entity.h"


// TODO: THE CONST-CORRECTNESS HERE IS UNNECSSARY - NOT SURE IF I LIKE IT
glm::mat4 buildTransformMatrix(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation);
void renderSkybox(const SkyboxData& skyboxData);
void renderSystem(const std::vector<uint32_t>& visibleEntities, const SparseSet<MaterialData>& materialSet,
                 const SparseSet<MeshData>& meshSet, const SparseSet<TransformComponent>& transformSet, const Framebuffer& framebuffer);
void drawToFramebuffer(const Framebuffer& framebuffer, GLuint quadVAO);
void renderText(const char* text, int textLength, const float xPos, const float yPos,
                const float size, const uint32_t screenWidth,
                const uint32_t screenHeight, TextRenderData& textRenderData);