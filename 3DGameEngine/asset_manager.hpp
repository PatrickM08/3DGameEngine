#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include "shader_s.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <array>
#include <vector>

struct Material {
	std::shared_ptr<Shader> shader;
	uint32_t textureCount;
	std::array<unsigned int, 4> texture;
};

/*
* Could be an idea in the future, waste of time now though
struct VerticesData {
	uint32_t size;
	uint32_t stride;
	std::vector<float> vertices;
	std::vector<int> vertexAttribStrides;
	GLenum drawType;
};
*/

struct MeshBuffers {
	uint32_t VBO;
	uint32_t VAO;
	uint32_t instanceVBO;

	const void deleteBuffers() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &instanceVBO);
	}
};

void createQuadBuffers(MeshBuffers& quad);
void createCubeBuffers(MeshBuffers& cube);
void createLightBuffers(MeshBuffers& light);
void createSkyboxbuffers(MeshBuffers& skybox);
#endif