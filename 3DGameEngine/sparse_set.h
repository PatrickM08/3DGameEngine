#pragma once
#include <cstdint>
#include <algorithm>
#include "entity.h"
#include "asset_manager.h"

#define MAX_ENTITIES 6000
#define CAPACITY_TRANSFORM 6000
#define CAPACITY_MESH 6000
#define CAPACITY_MATERIAL 6000
#define CAPACITY_RENDERABLE 6000
#define CAPACITY_VELOCITY 6000
#define CAPACITY_COLLISION 6000
#define CAPACITY_DYNAMIC 16
#define CAPACITY_BULLET 128
#define CAPACITY_CAMERA 4
#define CAPACITY_POINT_LIGHT 256
#define CAPACITY_HEALTH 4
#define CAPACITY_SPEED 16
#define CAPACITY_ROT_SPEED 16
#define CAPACITY_PATROL 16
#define CAPACITY_INPUT_MAP 16
#define CAPACITY_INPUT_WORLD 16
#define CAPACITY_INPUT_TANK 16
#define CAPACITY_INPUT_NOCLIP 16
#define CAPACITY_NAME 256

constexpr size_t ARENA_SIZE =
    CAPACITY_TRANSFORM * (sizeof(TransformComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_MESH * (sizeof(MeshData) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_MATERIAL * (sizeof(MaterialData) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_RENDERABLE * (sizeof(RenderableTag) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_VELOCITY * (sizeof(VelocityComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_COLLISION * (sizeof(CollisionComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_DYNAMIC * (sizeof(DynamicTag) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_BULLET * (sizeof(BulletTag) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_CAMERA * (sizeof(CameraComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_POINT_LIGHT * (sizeof(PointLightComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_HEALTH * (sizeof(HealthComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_SPEED * (sizeof(SpeedComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_ROT_SPEED * (sizeof(RotationSpeedComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_PATROL * (sizeof(PatrolComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_INPUT_MAP * (sizeof(InputMapComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_INPUT_WORLD * (sizeof(PlayerInputWorldTag) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_INPUT_TANK * (sizeof(PlayerInputTankTag) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_INPUT_NOCLIP * (sizeof(PlayerInputNoClipTag) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    CAPACITY_NAME * (sizeof(NameComponent) + sizeof(uint32_t)) + MAX_ENTITIES * sizeof(uint32_t) +
    1024; // in case of padding

struct Arena {
    uint8_t* base;
    size_t offset = 0;
    size_t capacity;

    void init(size_t bytes) {
        base = (uint8_t*)malloc(bytes);
        capacity = bytes;
    }

    void* alloc(size_t bytes, size_t alignment) {
        offset = (offset + alignment - 1) & ~(alignment - 1);
        void* ptr = base + offset;
        offset += bytes;
        return ptr;
    }
};

#define INVALID_INDEX UINT32_MAX

template <typename Component>
class SparseSet {
public:
    uint32_t* sparse;
    Component* dense;
    uint32_t* entities;
    uint32_t entityCount = 0;
    uint32_t denseCapacity;

    void init(Arena& arena, uint32_t capacity) {
        denseCapacity = capacity;
        sparse = (uint32_t*)arena.alloc(MAX_ENTITIES * sizeof(uint32_t), alignof(uint32_t));
        dense = (Component*)arena.alloc(capacity * sizeof(Component), alignof(Component));
        entities = (uint32_t*)arena.alloc(capacity * sizeof(uint32_t), alignof(uint32_t));
        std::fill(sparse, sparse + MAX_ENTITIES, INVALID_INDEX);
    }

    bool hasComponent(uint32_t entityID) const {
        if (entityID >= MAX_ENTITIES) return false;
        uint32_t denseIndex = sparse[entityID];
        return denseIndex != INVALID_INDEX;
    }

    // Read component
    const Component& getComponent(uint32_t entityID) const {
        uint32_t denseIndex = sparse[entityID];
        return dense[denseIndex];
    }

    // Write to component
    Component& getComponent(uint32_t entityID) {
        uint32_t denseIndex = sparse[entityID];
        return dense[denseIndex];
    }

    void add(uint32_t entityID, const Component& component) {
        if (entityCount >= denseCapacity) return;
        dense[entityCount] = component;
        sparse[entityID] = entityCount;
        entities[entityCount] = entityID;
        ++entityCount;
    }

    void remove(uint32_t entityID) {
        if (entityID >= MAX_ENTITIES || sparse[entityID] == INVALID_INDEX) {
            return;
        }
        uint32_t denseIndex = sparse[entityID];
        uint32_t denseLast = entityCount - 1;
        uint32_t entitiesLast = entities[denseLast];
        dense[denseIndex] = dense[denseLast];
        entities[denseIndex] = entitiesLast;
        sparse[entitiesLast] = denseIndex;
        --entityCount;
        sparse[entityID] = INVALID_INDEX;
    }

    void rebuildSparse() {
        std::fill(sparse, sparse + MAX_ENTITIES, INVALID_INDEX);
        for (uint32_t i = 0; i < entityCount; i++) {
            sparse[entities[i]] = i;
        }
    }
};