#pragma once
#include <cstdint>
#include <algorithm>

// WE SHOULD MAKE THIS DYNAMIC PER ENTITY
static constexpr uint32_t MAX_ENTITIES = 6000;
static constexpr uint32_t INVALID_INDEX = UINT32_MAX;

template <typename Component>
class SparseSet {
public:
    uint32_t sparse[MAX_ENTITIES];
    Component dense[MAX_ENTITIES];
    uint32_t entities[MAX_ENTITIES];
    uint32_t entityCount;

    SparseSet() {
        std::fill(sparse, sparse + MAX_ENTITIES, INVALID_INDEX);
        entityCount = 0;
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
        if (entityCount >= MAX_ENTITIES - 1) return;
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