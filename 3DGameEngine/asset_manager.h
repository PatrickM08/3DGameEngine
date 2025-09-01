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

class AssetManager {
public:
	std::vector<MeshData> meshes;
	std::vector<MaterialData> materials;

private:
	void initVertexAttributes(bool hasTexCoords, bool hasNormals);
	std::vector<float> parseOBJFile(const char* path, uint32_t& vertexCount, bool& hasTexCoords, bool& hasNormals);
	GLuint loadTexture(char const* path);
	GLuint loadCubemap(const std::vector<std::string>& faces);

public:
	AssetManager();
	std::vector<MeshData> loadMeshes(const char* path);
	std::vector<MaterialData> loadMaterials(const char* path);
	const MeshData& getMesh(uint32_t handle);
	const MaterialData& getMaterial(uint32_t handle);
};