#include "asset_manager.h"
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <iostream>
#include "tiny_obj_loader.h"

std::string getPath(const std::string &relativePath)
{
    #ifdef PROJECT_SOURCE_DIR
        return std::string(PROJECT_SOURCE_DIR) + "/" + relativePath;
    #else
        return relativePath;
    #endif
}

std::vector<float> AssetManager::parseOBJFile(const std::string& path, uint32_t& vertexCount) {
    tinyobj::ObjReader reader;
    std::vector<float> vertices;

    if (!reader.ParseFromFile(path)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                vertices.insert(vertices.end(), { vx, vy, vz });
                vertexCount++;

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    vertices.insert(vertices.end(), { nx, ny, nz });
                }
                else {
                    vertices.insert(vertices.end(), { 0, 1, 0 });
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vertices.insert(vertices.end(), { tx, ty});
                }
                else {
                    vertices.insert(vertices.end(), { 0,0 });
                }
            }
            index_offset += fv;
        }
    }
    int count = 0;
    for (float i : vertices) {
        if (count % 8 == 0) {
            std::cout << "\n";
        }
        std::cout << i << " ";
        count++;
    }
    std::cout << std::endl;
    return vertices;
}

/*
std::vector<float> AssetManager::parseOBJFile(const char* path, uint32_t& vertexCount, bool& hasTexCoords, bool& hasNormals) {
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
            hasTexCoords = true;
            float x, y;
            stream >> x >> y;
            texCoords.push_back(x);
            texCoords.push_back(y);
        }
        else if (prefix == "vn") {
            onlyPos = false;
            hasNormals = true;
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
*/

AssetManager::AssetManager() : meshes(loadMeshes("meshes.txt")), materials(loadMaterials("materials.txt")) {}

std::vector<MeshData> AssetManager::loadMeshes(const char* path) {
	std::vector<MeshData> meshes;
	std::ifstream file(getPath(path));
	if (!file.is_open()) {
		throw std::runtime_error("Error opening mesh definition file.");
	}
	std::string line;
	while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
		std::istringstream stream(line);
        uint32_t meshHandle;
        std::string objPath;
        std::string drawUsageString;
        stream >> meshHandle >> objPath >> drawUsageString;
        GLenum drawUsage = GL_STATIC_DRAW;
        if (drawUsageString == "GL_DYNAMIC_DRAW") drawUsage = GL_DYNAMIC_DRAW;
        else if (drawUsageString == "GL_STREAM_DRAW") drawUsage = GL_STREAM_DRAW;
        else if (drawUsageString == "GL_STATIC_DRAW") drawUsage = GL_STATIC_DRAW;
        uint32_t vertexCount = 0;
        std::vector<float> vertices = parseOBJFile(getPath(objPath), vertexCount);
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), drawUsage);
        // The vertex format for static mesh vertex buffers is standardised (vx,vy,vz,nx,ny,nz,tx,ty)
        size_t vertexAttribStride = 8 * sizeof(float);  
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexAttribStride, (void*)0);
        glEnableVertexAttribArray(0);
        size_t normalsOffset = 3 * sizeof(float);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexAttribStride, (void*)normalsOffset);
        glEnableVertexAttribArray(1);
        size_t texCoordsOffset = 6 * sizeof(float);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertexAttribStride, (void*)texCoordsOffset);
        glEnableVertexAttribArray(2);
        meshes.emplace_back();
        auto& mesh = meshes.back();
        mesh.handle = meshHandle;
        mesh.vao = VAO;
        mesh.vertexCount = vertexCount;
	}
    return meshes;
}


std::vector<MaterialData> AssetManager::loadMaterials(const char* path) {
    std::vector<MaterialData> materials;
    std::ifstream file(getPath(path));
    if (!file.is_open()) {
        throw std::runtime_error("Error opening material definition file.");
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream stream(line);
        std::string prefix;
        stream >> prefix;
        if (prefix == "material") {
            materials.emplace_back();
            auto& material = materials.back();
            uint32_t materialHandle;
            stream >> materialHandle;
            material.handle = materialHandle;
            std::string vertexShader;
            std::string fragmentShader;
            stream >> vertexShader >> fragmentShader;
            Shader shader(vertexShader.c_str(), fragmentShader.c_str());
            material.shader = shader;
        }
        else if (prefix == "lit") {
            if (!materials.empty()) {
                auto& material = materials.back();
                std::string restOfLine;
                std::getline(stream, restOfLine);
                std::replace(restOfLine.begin(), restOfLine.end(), ',', ' ');
                std::istringstream lightStream(restOfLine);
                lightStream >> material.ambient.x >> material.ambient.y >> material.ambient.z
                    >> material.diffuse.x >> material.diffuse.y >> material.diffuse.z
                    >> material.specular.x >> material.specular.y >> material.specular.z
                    >> material.shininess;
            }
        }
        else if (prefix == "textures") {
            if (!materials.empty()) {
                auto& material = materials.back();
                std::string restOfLine;
                std::getline(stream, restOfLine);
                std::replace(restOfLine.begin(), restOfLine.end(), ',', ' ');
                std::istringstream textureStream(restOfLine);
                std::string texturePath;
                std::string textureTargetString;
                while (textureStream >> texturePath >> textureTargetString) {
                    material.textures.emplace_back();
                    auto& texture = material.textures.back();
                    texture.id = loadTexture(getPath(texturePath));
                    GLenum textureTarget = GL_TEXTURE_2D;
                    if (textureTargetString == "GL_TEXTURE_CUBE_MAP") textureTarget = GL_TEXTURE_CUBE_MAP;
                    else if (textureTargetString == "GL_TEXTURE_2D") textureTarget = GL_TEXTURE_2D;
                    texture.target = textureTarget;
                }
            }
        }
        else if (prefix == "cubemap") {
            if (!materials.empty()) {
                auto& material = materials.back();
                std::string restOfLine;
                std::getline(stream, restOfLine);
                std::replace(restOfLine.begin(), restOfLine.end(), ',', ' ');
                std::istringstream textureStream(restOfLine);
                std::vector<std::string> faces;
                std::string face;
                for (int i = 0; i < 6; i++) {
                    textureStream >> face;
                    faces.push_back(getPath(face));
                }
                material.textures.emplace_back();
                auto& texture = material.textures.back();
                texture.id = loadCubemap(faces);
                texture.target = GL_TEXTURE_CUBE_MAP;
            }
        }
    };
    return materials;
}

GLuint AssetManager::loadTexture(const std::string& path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

GLuint AssetManager::loadCubemap(const std::vector<std::string>& faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int count = 0;
    int width, height, nrChannels;
    for (const std::string& face : faces)
    {
        unsigned char* data = stbi_load(face.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + count, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << face << std::endl;
            stbi_image_free(data);
        }
        count++;
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

const MeshData& AssetManager::getMesh(uint32_t handle) {
    return meshes[handle];
}

MaterialData& AssetManager::getMaterial(uint32_t handle) {
    return materials[handle];
}