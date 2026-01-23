#include "asset_manager.h"
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <limits>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <iostream>
#include "tiny_obj_loader.h"
#include <bit>

// TODO: VERY VERY BAD - FIGURE THIS OUT
std::string getPath(const std::string& relativePath) {
#ifdef PROJECT_SOURCE_DIR
    return std::string(PROJECT_SOURCE_DIR) + "/" + relativePath;
#else
    return relativePath;
#endif
}

std::vector<float> AssetManager::parseOBJFile(const std::string& path, uint32_t& vertexCount, AABB& localAABB) {
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
                if (vx < localAABB.minX) localAABB.minX = vx;
                else if (vx > localAABB.maxX) localAABB.maxX = vx;
                if (vy < localAABB.minY) localAABB.minY = vy;
                else if (vy > localAABB.maxY) localAABB.maxY = vy;
                if (vz < localAABB.minZ) localAABB.minZ = vz;
                else if (vz > localAABB.maxZ) localAABB.maxZ = vz;
                vertices.insert(vertices.end(), {vx, vy, vz});
                vertexCount++;

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    vertices.insert(vertices.end(), {nx, ny, nz});
                } else {
                    vertices.insert(vertices.end(), {0, 1, 0});
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vertices.insert(vertices.end(), {tx, ty});
                } else {
                    vertices.insert(vertices.end(), {0, 0});
                }
            }
            index_offset += fv;
        }
    }
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

AssetManager::AssetManager()
    : meshes(loadMeshes("meshes.txt")), materials(loadMaterials()) {}

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
        if (drawUsageString == "GL_DYNAMIC_DRAW")
            drawUsage = GL_DYNAMIC_DRAW;
        else if (drawUsageString == "GL_STREAM_DRAW")
            drawUsage = GL_STREAM_DRAW;
        else if (drawUsageString == "GL_STATIC_DRAW")
            drawUsage = GL_STATIC_DRAW;
        uint32_t vertexCount = 0;
        float maxFloat = std::numeric_limits<float>::max();
        AABB localAABB{maxFloat, maxFloat, maxFloat, -maxFloat, -maxFloat, -maxFloat};
        std::vector<float> vertices = parseOBJFile(getPath(objPath), vertexCount, localAABB);
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
        mesh.localAABB = localAABB;
    }
    return meshes;
}

// TODO: THIS IS AN ABSOLUTE MESS - NEEDS TO BE COMPLETELY REWORKED - START FROM A CODE SOLUTION AND THEN POTENTIALLY ADD A FILE SOLUTION
std::vector<MaterialData> AssetManager::loadMaterials() {
    std::vector<MaterialData> materials;
    std::vector<MaterialSSBOData> materialSSBOData;
    uint64_t cubeDiffuse = loadTexture(getPath("container2.png").c_str());
    uint64_t cubeSpecular = loadTexture(getPath("container2_specular.png").c_str());
    // THIS NEEDS TO BE IN THE GLOBAL UBO - SO PROBABLY A SEPERATE FUNCTION
    std::string paths[6] = {
        getPath("right.jpg"), getPath("left.jpg"),
        getPath("top.jpg"), getPath("bottom.jpg"),
        getPath("front.jpg"), getPath("back.jpg")};

    const char* cubemapTexturePaths[6] = {
        paths[0].c_str(), paths[1].c_str(), paths[2].c_str(),
        paths[3].c_str(), paths[4].c_str(), paths[5].c_str()};

    uint64_t skyboxCubemap = loadCubemap(cubemapTexturePaths);

    int materialCount = 0;
    MaterialSSBOData cubeMaterial = {cubeDiffuse, cubeSpecular, 32.0f};
    MaterialSSBOData noMaterial = {0, 0, 0.0f};
    materialSSBOData.push_back(cubeMaterial);
    materialSSBOData.push_back(noMaterial);
    materials.emplace_back(cubeMaterial, 0, createShaderProgram("cube.vs", "cube.fs"), 0);
    materials.emplace_back(noMaterial, 0, createShaderProgram("vshader.vs", "fshader.fs"), 1);

    // TODO: THIS SHOULD BE PASSED OUT - THIS WHOLE STRUCTURE IS A MESS WITH THE ASSET MANAGER
    GLuint materialSSBO;
    glGenBuffers(1, &materialSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);

    glBufferData(GL_SHADER_STORAGE_BUFFER, materialSSBOData.size() * sizeof(MaterialSSBOData), materialSSBOData.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialSSBO);

    return materials;
}

// TODO: ADD A FALL BACK TEXTURE
uint64_t AssetManager::loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 4);
    if (data) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        int mipMapLevel = std::bit_width((uint32_t)std::max(width, height));
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    
    uint64_t textureHandle = glGetTextureHandleARB(textureID);
    glMakeTextureHandleResidentARB(textureHandle);
    return textureHandle;
}

uint64_t AssetManager::loadCubemap(const char* (&faces)[6]) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrComponents;
    for (int i = 0; i < 6; ++i) {
        unsigned char* data = stbi_load(faces[i], &width, &height, &nrComponents, 4);
        if (data) {
            if (i == 0) {
                int mipMapLevel = std::bit_width((uint32_t)std::max(width, height));
                glTexStorage2D(GL_TEXTURE_CUBE_MAP, mipMapLevel, GL_RGBA8, width, height);
            }
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    uint64_t textureHandle = glGetTextureHandleARB(textureID);
    glMakeTextureHandleResidentARB(textureHandle);
    return textureHandle;
}

// TODO: UNBIND AFTER MESH AND TEXTURE STUFF
// TODO: CHANGE VERTEX COUNT TO INDEX COUNT
// TODO: CHANGE THE MESH AND MATERIAL DATA HANDLES TO 16 BIT
// TODO: FIX THE LOCALAABB AND WORLD AABB
// TODO: Figure out if this is correct - so we should have unit primitives lazy loaded in the asset manager but we need to improve the asset manager
// we also need to look into gl_vertexID usage, we also need to look into deleting buffers when no longer used.
// Assumes centre as actual position
// TODO: PRIMITIVES RIGHT NOW ARE THE ONLY THING USING EBOS AND VERTEX COUNT IS ACTUALLY INDEX COUNT, WILL FIX AFTER WE FIX ASSET LOADING
/*
MeshData createUnitCubePrimitive(std::vector<MeshData>& meshes) {
    float xOffset = 0.5f;
    float yOffset = 0.5f;
    float zOffset = 0.5f;
    float vertices[24] = {-xOffset, -yOffset, zOffset,
                          xOffset, -yOffset, zOffset,
                          xOffset, yOffset, zOffset,
                          -xOffset, yOffset, zOffset,
                          -xOffset, -yOffset, -zOffset,
                          xOffset, -yOffset, -zOffset,
                          xOffset, yOffset, -zOffset,
                          -xOffset, yOffset, -zOffset};

    unsigned int indices[36] = {0, 1, 2, 2, 3, 0,
                              1, 5, 6, 6, 2, 1,
                              7, 6, 5, 5, 4, 7,
                              4, 0, 3, 3, 7, 4,
                              4, 5, 1, 1, 0, 4,
                              3, 2, 6, 6, 7, 3};
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // TODO: THIS WOULD HAVE TO BE CHANGED DEPENDING ON THE SHADER AND NORMALS AND STUFF - SHOULD PROBABLY BE STANDARDISED - ALSO SEE IF SHOULD BE STATIC
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    uint32_t numberOfMeshes = static_cast<uint32_t>(meshes.size());
    meshes.emplace_back(numberOfMeshes, VAO, 36, AABB{-xOffset, -yOffset, -zOffset, xOffset, yOffset, zOffset});
    return meshes.back();
}
*/

MeshData createUnitCubePrimitive(std::vector<MeshData>& meshes) {
    float xOffset = 0.5f;
    float yOffset = 0.5f;
    float zOffset = 0.5f;
    float vertices[] = {
        -xOffset, -yOffset, zOffset,
        xOffset, -yOffset, zOffset,
        xOffset, yOffset, zOffset,
        -xOffset, -yOffset, zOffset,
        xOffset, yOffset, zOffset,
        -xOffset, yOffset, zOffset,

        xOffset, -yOffset, -zOffset,
        -xOffset, -yOffset, -zOffset,
        -xOffset, yOffset, -zOffset,
        xOffset, -yOffset, -zOffset,
        -xOffset, yOffset, -zOffset,
        xOffset, yOffset, -zOffset,

        -xOffset, -yOffset, -zOffset,
        -xOffset, -yOffset, zOffset,
        -xOffset, yOffset, zOffset,
        -xOffset, -yOffset, -zOffset,
        -xOffset, yOffset, zOffset,
        -xOffset, yOffset, -zOffset,

        xOffset, -yOffset, zOffset,
        xOffset, -yOffset, -zOffset,
        xOffset, yOffset, -zOffset,
        xOffset, -yOffset, zOffset,
        xOffset, yOffset, -zOffset,
        xOffset, yOffset, zOffset,

        -xOffset, yOffset, zOffset,
        xOffset, yOffset, zOffset,
        xOffset, yOffset, -zOffset,
        -xOffset, yOffset, zOffset,
        xOffset, yOffset, -zOffset,
        -xOffset, yOffset, -zOffset,

        -xOffset, -yOffset, -zOffset,
        xOffset, -yOffset, -zOffset,
        xOffset, -yOffset, zOffset,
        -xOffset, -yOffset, -zOffset,
        xOffset, -yOffset, zOffset,
        -xOffset, -yOffset, zOffset};

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // TODO: THIS WOULD HAVE TO BE CHANGED DEPENDING ON THE SHADER AND NORMALS AND STUFF - SHOULD PROBABLY BE STANDARDISED - ALSO SEE IF SHOULD BE STATIC
    // TOO DEPENDENT ON SHADER
    glBufferData(GL_ARRAY_BUFFER, 36 * 3 * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    uint32_t numberOfMeshes = static_cast<uint32_t>(meshes.size());
    meshes.emplace_back(numberOfMeshes, VAO, 36, AABB{-xOffset, -yOffset, -zOffset, xOffset, yOffset, zOffset});
    return meshes.back();
}