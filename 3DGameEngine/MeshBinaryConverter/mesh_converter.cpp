#define _CRT_SECURE_NO_WARNINGS
#include "tiny_obj_loader.h"
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <limits>

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float tx, ty;

    bool operator==(const Vertex& other) const {
        return memcmp(this, &other, sizeof(Vertex)) == 0;
    }
};

struct VertexHash {
    size_t operator()(const Vertex& v) const {
        size_t hash = 0;
        const uint32_t* data = reinterpret_cast<const uint32_t*>(&v);
        for (int i = 0; i < 8; ++i) {
            hash ^= data[i] + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

struct AABB {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

struct MeshFileHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    AABB localAABB;
};

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: mesh_converter input.obj output.mesh\n");
        return 1;
    }

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(argv[1])) {
        printf("Failed to parse %s: %s\n", argv[1], reader.Error().c_str());
        return 1;
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t, VertexHash> uniqueVerts;

    float maxF = std::numeric_limits<float>::max();
    AABB aabb = {maxF, maxF, maxF, -maxF, -maxF, -maxF};

    for (size_t s = 0; s < shapes.size(); ++s) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
            size_t fv = shapes[s].mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shapes[s].mesh.indices[indexOffset + v];

                Vertex vert = {};
                vert.px = attrib.vertices[3 * idx.vertex_index + 0];
                vert.py = attrib.vertices[3 * idx.vertex_index + 1];
                vert.pz = attrib.vertices[3 * idx.vertex_index + 2];

                if (idx.normal_index >= 0) {
                    vert.nx = attrib.normals[3 * idx.normal_index + 0];
                    vert.ny = attrib.normals[3 * idx.normal_index + 1];
                    vert.nz = attrib.normals[3 * idx.normal_index + 2];
                } else {
                    vert.ny = 1.0f;
                }

                if (idx.texcoord_index >= 0) {
                    vert.tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    vert.ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                }

                if (vert.px < aabb.minX)
                    aabb.minX = vert.px;
                if (vert.px > aabb.maxX)
                    aabb.maxX = vert.px;
                if (vert.py < aabb.minY)
                    aabb.minY = vert.py;
                if (vert.py > aabb.maxY)
                    aabb.maxY = vert.py;
                if (vert.pz < aabb.minZ)
                    aabb.minZ = vert.pz;
                if (vert.pz > aabb.maxZ)
                    aabb.maxZ = vert.pz;

                auto it = uniqueVerts.find(vert);
                if (it != uniqueVerts.end()) {
                    indices.push_back(it->second);
                } else {
                    uint32_t newIndex = (uint32_t)vertices.size();
                    vertices.push_back(vert);
                    uniqueVerts[vert] = newIndex;
                    indices.push_back(newIndex);
                }
            }
            indexOffset += fv;
        }
    }

    MeshFileHeader header;
    header.vertexCount = (uint32_t)vertices.size();
    header.indexCount = (uint32_t)indices.size();
    header.localAABB = aabb;

    FILE* out = fopen(argv[2], "wb");
    fwrite(&header, sizeof(MeshFileHeader), 1, out);
    fwrite(vertices.data(), sizeof(Vertex), vertices.size(), out);
    fwrite(indices.data(), sizeof(uint32_t), indices.size(), out);
    fclose(out);

    printf("Converted: %u unique verts, %u indices\n",
           header.vertexCount, header.indexCount);

    return 0;
}