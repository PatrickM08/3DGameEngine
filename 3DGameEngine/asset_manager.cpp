#include "asset_manager.h"
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

std::vector<float> parseOBJFile(const char* path, uint32_t& vertexCount) {
    bool onlyPos = true;
    std::vector<float> meshData;
    std::vector<float> positions;
    std::vector<float> texCoords;
    std::vector<float> normals;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening obj file.");
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string prefix;
        stream >> prefix;

        if (prefix == "v") {
            float x, y, z;
            stream >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        else if (prefix == "vt") {
            onlyPos = false;
            float x, y;
            stream >> x >> y;
            texCoords.push_back(x);
            texCoords.push_back(y);
        }
        else if (prefix == "vn") {
            onlyPos = false;
            float x, y, z;
            stream >> x >> y >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }
        else if (prefix == "f") {
            std::string vertex;
            while (stream >> vertex) {
                vertexCount++;
                std::replace(vertex.begin(), vertex.end(), '/', ' ');
                std::istringstream vstream(vertex);
                int v;
                vstream >> v;
                v = (v - 1) * 3;
                meshData.push_back(positions[v]);
                meshData.push_back(positions[v + 1]);
                meshData.push_back(positions[v + 2]);
                if (!onlyPos) {
                    int t, n;
                    vstream >> t >> n;

                    t = (t - 1) * 2;
                    n = (n - 1) * 3;

                    meshData.push_back(texCoords[t]);
                    meshData.push_back(texCoords[t + 1]);
                    meshData.push_back(normals[n]);
                    meshData.push_back(normals[n + 1]);
                    meshData.push_back(normals[n + 2]);
                }
            }
        }
    }
    return meshData;
}

AssetManager::AssetManager() : meshes(loadMeshes("meshes.txt")) {}

std::vector<MeshData> AssetManager::loadMeshes(const char* path) {
	std::vector<MeshData> meshes;
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Error opening obj file.");
	}
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream stream(line);
        uint32_t meshHandle;
        std::string objPath;
        stream >> meshHandle >> objPath;
        MeshData mesh;
        mesh.handle = meshHandle;
        uint32_t vertexCount = 0;
        std::vector<float> vertices = parseOBJFile(path, vertexCount);
        mesh.vertexCount = vertexCount;

	}
} 