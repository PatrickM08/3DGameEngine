#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader_s.h"

struct Texture {
	GLuint id;
	GLenum target;
};

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

struct MaterialData {
	uint32_t handle;
    Shader shader;
	float shininess;
	std::vector<Texture> textures;
};

std::string getPath(const std::string &relativePath);
MeshData createUnitCubePrimitive(std::vector<MeshData>& meshes);

class AssetManager {
private:
	std::vector<MeshData> loadMeshes(const char*);
	std::vector<MaterialData> loadMaterials(const char* path);
    std::vector<float> parseOBJFile(const std::string& path, uint32_t& vertexCount, AABB& localAABB);
	GLuint loadTexture(const std::string& path);
	GLuint loadCubemap(const std::vector<std::string>& faces);

public:
	AssetManager();
	const MeshData& getMesh(uint32_t handle);
	MaterialData& getMaterial(uint32_t handle);
    std::vector<MeshData> meshes;
    std::vector<MaterialData> materials;
};