#pragma once
#include <vector>
#include <cstdint>
#include <memory>
#include <algorithm>

// WE SHOULD MAKE THIS DYNAMIC PER ENTITY
static constexpr uint32_t MAX_ENTITIES = 200000;
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
};


inline const uint32_t PAGE_SIZE = 512;
struct SparseSetPage {
    uint32_t data[PAGE_SIZE];
    SparseSetPage() {
        std::fill(std::begin(data), std::end(data), UINT32_MAX);
    } // Required due to possible garbage data access
};

// PAGED SHOULD ONLY BE USED FOR INIT ENTITIES UNTIL WE HAVE VERSION CONTROL
template <typename Component>
class PagedSparseSet {
public:
    std::vector<std::unique_ptr<SparseSetPage>> sparsePages;
    std::vector<Component> dense;
    std::vector<uint32_t> entities;

    bool hasComponent(uint32_t entityID) const {
        uint32_t pageIndex = entityID / PAGE_SIZE;
        if (pageIndex >= sparsePages.size() || sparsePages[pageIndex] == nullptr) return false;
        uint32_t denseIndex = sparsePages[pageIndex]->data[entityID % PAGE_SIZE];
        return denseIndex != UINT32_MAX && denseIndex < dense.size() && entities[denseIndex] == entityID;
    }

    const Component& getComponent(uint32_t entityID) const {
        uint32_t pageIndex = entityID / PAGE_SIZE;
        uint32_t denseIndex = sparsePages[pageIndex]->data[entityID % PAGE_SIZE];
        return dense[denseIndex];
    }

    Component& getComponent(uint32_t entityID) {
        uint32_t pageIndex = entityID / PAGE_SIZE;
        uint32_t denseIndex = sparsePages[pageIndex]->data[entityID % PAGE_SIZE];
        return dense[denseIndex];
    }

    void add(uint32_t entityID, const Component& component) {
        uint32_t pageIndex = entityID / PAGE_SIZE;
        if (pageIndex >= sparsePages.size()) {
            sparsePages.resize(pageIndex + 1);
        }
        if (sparsePages[pageIndex] == nullptr) {
            sparsePages[pageIndex] = std::make_unique<SparseSetPage>();
        }
        dense.push_back(component);
        sparsePages[pageIndex]->data[entityID % PAGE_SIZE] = dense.size() - 1;
        entities.push_back(entityID);
    }

    void remove(uint32_t entityID) {
        uint32_t pageIndex = entityID / PAGE_SIZE;
        uint32_t denseIndex = sparsePages[pageIndex]->data[entityID % PAGE_SIZE];
        uint32_t denseLast = dense.size() - 1;
        uint32_t entitiesLast = entities[denseLast];
        dense[denseIndex] = dense[denseLast];
        entities[denseIndex] = entitiesLast;
        uint32_t lastEntityPageIndex = entitiesLast / PAGE_SIZE;
        sparsePages[lastEntityPageIndex]->data[entitiesLast % PAGE_SIZE] = denseIndex;
        dense.pop_back();
        entities.pop_back();
    }

    const std::vector<uint32_t>& getEntities() const {
        return entities;
    }
};
