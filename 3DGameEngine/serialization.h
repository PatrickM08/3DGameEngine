#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include "sparse_set.h"

struct ECS;

template <typename T>
void writeSet(FILE* f, SparseSet<T>& set) {
    fwrite(&set.entityCount, sizeof(uint32_t), 1, f);
    fwrite(set.entities, sizeof(uint32_t), set.entityCount, f);
    fwrite(set.dense, sizeof(T), set.entityCount, f);
}

template <typename T>
void readSet(FILE* f, SparseSet<T>& set) {
    fread(&set.entityCount, sizeof(uint32_t), 1, f);
    fread(set.entities, sizeof(uint32_t), set.entityCount, f);
    fread(set.dense, sizeof(T), set.entityCount, f);
    set.rebuildSparse();
}

void saveScene(ECS& scene, const char* path);
void loadScene(ECS& scene, const char* path);