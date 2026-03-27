#pragma once
#include <string>
#include <cstdint>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct AABB {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

struct MeshData {
    uint32_t handle;
    uint32_t vao;
    GLsizei vertexCount;
    GLsizei indexCount;
    AABB localAABB;
};

struct MeshFileHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    AABB localAABB;
};

struct MaterialSSBOData {
    glm::vec4 colourAndShine;
    uint64_t diffuseTextureHandle;
    uint64_t specularTextureHandle;
};

struct MaterialData {
    uint32_t shaderID;
    uint16_t materialSSBOIndex;
};

struct MeshBuffer {
    static constexpr uint8_t capacity = 255;
    uint8_t size = 0;
    MeshData buffer[capacity];
};

struct MaterialBuffer {
    static constexpr uint8_t capacity = 255;
    uint8_t size = 0;
    MaterialData buffer[capacity];
};

struct MaterialSSBODataBuffer {
    static constexpr uint8_t capacity = 255;
    uint8_t size = 0;
    MaterialSSBOData buffer[capacity];
};

uint64_t loadTexture(const char* path);
uint64_t loadCubemap(const char* (&faces)[6]);
uint64_t loadSkyboxCubemap();
void updateMaterialColour(MaterialData& materialData, MaterialSSBOData& materialSSBOData, glm::vec3 newColor, uint32_t ssbo);
uint64_t createDefaultTexture();
uint32_t initMaterialSSBO(MaterialSSBODataBuffer& materialSSBODataBuffer);
void initDefaultMaterials(MaterialBuffer& materialBuffer, MaterialSSBODataBuffer& materialSSBODataBuffer);
void initMeshes(MeshBuffer& meshBuffer);
uint32_t createUnitCubePrimitive(MeshBuffer& meshBuffer);
