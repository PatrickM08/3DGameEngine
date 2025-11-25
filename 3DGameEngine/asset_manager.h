#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader_s.h"

class Texture {
public:
	GLuint id;
	GLenum target;
public:
	
};

struct MeshData {
	uint32_t handle;
	GLuint vao;
	GLsizei vertexCount;
};

struct MaterialData {
	uint32_t handle;
	Shader shader;
	glm::vec3 ambient, diffuse, specular;
	float shininess;
	std::vector<Texture> textures;
};

std::string getPath(const std::string &relativePath);

class AssetManager {
private:
	std::vector<MeshData> meshes;
	std::vector<MaterialData> materials;

private:
	std::vector<MeshData> loadMeshes(const char*);
	std::vector<MaterialData> loadMaterials(const char* path);
	std::vector<float> parseOBJFile(const std::string& path, uint32_t& vertexCount);
	GLuint loadTexture(const std::string& path);
	GLuint loadCubemap(const std::vector<std::string>& faces);

public:
	AssetManager();
	const MeshData& getMesh(uint32_t handle);
	MaterialData& getMaterial(uint32_t handle);
};