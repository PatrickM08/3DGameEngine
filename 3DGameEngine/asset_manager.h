#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader_s.h"

struct AABB {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

struct MeshData {
	uint32_t handle;
	GLuint vao;
	GLsizei vertexCount;
    AABB localAABB;
};

struct MaterialSSBOData {
    uint64_t diffuseTextureHandle;
    uint64_t specularTextureHandle;
    float shininess;
};

struct MaterialData {
    MaterialSSBOData materialData;
	uint32_t handle;                // TODO: Not sure if I need both handle and materialSSBOIndex - which probably means I don't
    uint32_t shaderID;
    uint16_t materialSSBOIndex;
};

std::string getPath(const std::string &relativePath);
MeshData createUnitCubePrimitive(std::vector<MeshData>& meshes);

class AssetManager {
private:
	std::vector<MeshData> loadMeshes(const char*);
	std::vector<MaterialData> loadMaterials();
    std::vector<float> parseOBJFile(const std::string& path, uint32_t& vertexCount, AABB& localAABB);
	uint64_t loadTexture(const char* path);
    uint64_t loadCubemap(const char* (&faces)[6]);

public:
	AssetManager();
    std::vector<MeshData> meshes;
    std::vector<MaterialData> materials;
};