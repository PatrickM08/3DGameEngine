#define _CRT_SECURE_NO_WARNINGS
#include "asset_manager.h"
#include "stb_image.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <bit>
#include "shader_s.h"

// TODO: VERY VERY BAD - FIGURE THIS OUT
std::string getPath(const std::string& relativePath) {
#ifdef PROJECT_SOURCE_DIR
    return std::string(PROJECT_SOURCE_DIR) + "/" + relativePath;
#else
    return relativePath;
#endif
}

void loadBinaryMesh(const std::string& path, MeshBuffer& meshBuffer) {
    FILE* f = fopen(path.c_str(), "rb");
    MeshFileHeader header;
    fread(&header, sizeof(MeshFileHeader), 1, f);

    std::vector<float> vertices(header.vertexCount * 8);
    fread(vertices.data(), sizeof(float) * 8, header.vertexCount, f);

    std::vector<uint32_t> indices(header.indexCount);
    fread(indices.data(), sizeof(uint32_t), header.indexCount, f);
    fclose(f);

    uint32_t VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    MeshData mesh;
    mesh.handle = meshBuffer.size;
    mesh.vao = VAO;
    mesh.vertexCount = header.vertexCount;
    mesh.indexCount = header.indexCount;
    mesh.localAABB = header.localAABB;
    meshBuffer.buffer[meshBuffer.size++] = mesh;
}

void updateMaterialColour(MaterialData& materialData, MaterialSSBOData& materialSSBOData, glm::vec3 newColor, uint32_t ssbo) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    materialData.materialSSBOIndex * sizeof(MaterialSSBOData),
                    sizeof(glm::vec3),
                    &newColor);
    materialSSBOData.colourAndShine.x = newColor.x;
    materialSSBOData.colourAndShine.y = newColor.y;
    materialSSBOData.colourAndShine.z = newColor.z;
}

uint64_t loadSkyboxCubemap() {
    std::string paths[6] = {
        getPath("right.jpg"), getPath("left.jpg"),
        getPath("top.jpg"), getPath("bottom.jpg"),
        getPath("front.jpg"), getPath("back.jpg")};

    const char* cubemapTexturePaths[6] = {
        paths[0].c_str(), paths[1].c_str(), paths[2].c_str(),
        paths[3].c_str(), paths[4].c_str(), paths[5].c_str()};

    return loadCubemap(cubemapTexturePaths);
}

uint64_t createDefaultTexture() {
    uint32_t whiteTextureID;
    glGenTextures(1, &whiteTextureID);
    glBindTexture(GL_TEXTURE_2D, whiteTextureID);

    uint8_t whitePixel[] = {255, 255, 255, 255};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    uint64_t whiteTextureHandle = glGetTextureHandleARB(whiteTextureID);
    glMakeTextureHandleResidentARB(whiteTextureHandle);

    return whiteTextureHandle;
}

uint64_t loadTexture(const char* path) {
    uint32_t textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 4);
    if (data) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        int mipMapLevel = std::bit_width((uint32_t)std::max(width, height));
        glTexStorage2D(GL_TEXTURE_2D, mipMapLevel, GL_RGBA8, width, height);
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

uint64_t loadCubemap(const char* (&faces)[6]) {
    uint32_t textureID;
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

void initDefaultMaterials(MaterialBuffer& materialBuffer, MaterialSSBODataBuffer& materialSSBODataBuffer) {
    uint64_t defualtTexture = createDefaultTexture();
    uint64_t cubeDiffuse = loadTexture(getPath("container2.png").c_str());
    uint64_t cubeSpecular = loadTexture(getPath("container2_specular.png").c_str());

    uint32_t defualtShaderID = createShaderProgram("default_vertex.vs", "default_fragment.fs");

    MaterialSSBOData cubeMaterial = {glm::vec4(1.0f, 1.0f, 1.0f, 32.0f), cubeDiffuse, cubeSpecular};
    MaterialSSBOData noMaterial = {glm::vec4(0.2f, 0.2f, 0.2f, 0.0f), defualtTexture, defualtTexture};
    materialSSBODataBuffer.buffer[materialSSBODataBuffer.size++] = cubeMaterial; // Index 0
    materialSSBODataBuffer.buffer[materialSSBODataBuffer.size++] = noMaterial;   // Index 1 - need to find a better way to do this.
    materialBuffer.buffer[materialBuffer.size++] = MaterialData{defualtShaderID, 0};
    materialBuffer.buffer[materialBuffer.size++] = MaterialData{defualtShaderID, 1};
}

uint32_t initMaterialSSBO(MaterialSSBODataBuffer& materialSSBODataBuffer) {
    uint32_t materialSSBO;
    glGenBuffers(1, &materialSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);
    // MAYBE SHOULD BE STATIC - DEPENDS HOW OFTEN THE MATERIALS ARE CHANGED.
    glBufferData(GL_SHADER_STORAGE_BUFFER, materialSSBODataBuffer.capacity * sizeof(MaterialSSBOData), materialSSBODataBuffer.buffer, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialSSBO);
    return materialSSBO;
}

// TODO: FIX THIS
void initMeshes(MeshBuffer& meshBuffer) {
    std::string baseplate = "baseplate.mesh";
    loadBinaryMesh(baseplate, meshBuffer);
    std::string cube = "cube.mesh";
    loadBinaryMesh(cube, meshBuffer);
    std::string skybox = "skybox.mesh";
    loadBinaryMesh(skybox, meshBuffer);
}

uint32_t createUnitCubePrimitive(MeshBuffer& meshBuffer) {
    float h = 0.5f;

    float vertices[] = {
        -h, -h, h, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        h, -h, h, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        h, h, h, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -h, h, h, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

        h, -h, -h, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -h, -h, -h, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -h, h, -h, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        h, h, -h, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,

        -h, -h, -h, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -h, -h, h, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -h, h, h, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -h, h, -h, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        h, -h, h, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        h, -h, -h, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        h, h, -h, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        h, h, h, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        -h, h, h, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        h, h, h, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        h, h, -h, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -h, h, -h, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        -h, -h, -h, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        h, -h, -h, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        h, -h, h, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -h, -h, h, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f};

    uint32_t indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20};

    uint32_t VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    uint32_t handle = static_cast<uint32_t>(meshBuffer.size);

    MeshData mesh;
    mesh.handle = handle;
    mesh.vao = VAO;
    mesh.vertexCount = 24;
    mesh.indexCount = 36;
    mesh.localAABB = AABB{-h, -h, -h, h, h, h};
    meshBuffer.buffer[meshBuffer.size++] = mesh;

    return handle;
}