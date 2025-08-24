#pragma once
#include <unordered_map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct MeshData {
	uint32_t handle;
	GLuint vao;
	GLsizei vertexCount;
};

class AssetManager {
private:
	std::vector<MeshData> meshes;

public:
	AssetManager();
	std::vector<MeshData> loadMeshes(const char* path);
};