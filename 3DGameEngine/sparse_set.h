#pragma once
#include <vector>
#include <cstdint>
#include <memory>

// GET AND REMOVE BOTH ASSUME THAT HAS WILL BE CALLED FIRST, IT IS THE RESPONSIBILITY OF THE CALLER
// (the idea is getComponent and remove may be called in succession so we wouldn't want a redundant hasComponent check)

// Sparse vector should have a generous reserved size when the rough amount of entities are known to avoid heap allocations, 
// however in the context of a prototyping engine, rather than your own game, this may not be appropriate, 
// and is therefore left without reservation.
template <typename Component>
class SparseSet {
private:
	std::vector<uint32_t> sparse;
	std::vector<Component> dense;
	std::vector<uint32_t> entities;

	static constexpr uint32_t INVALID_INDEX = UINT32_MAX;

public:
	bool hasComponent(uint32_t entityID) const {
		if (entityID >= sparse.size()) return false;
		uint32_t denseIndex = sparse[entityID];
		return denseIndex < dense.size() && entities[denseIndex] == entityID;
	}

	Component& getComponent(uint32_t entityID) {
		uint32_t denseIndex = sparse[entityID];
		return dense[denseIndex];
	}

	void add(uint32_t entityID, const Component& component) {
		if (entityID >= sparse.size()) {
			sparse.resize(entityID + 1, INVALID_INDEX);
		}
		dense.push_back(component);
		sparse[entityID] = dense.size() - 1;
		entities.push_back(entityID);
	}

	void remove(uint32_t entityID) {
		if (entityID >= sparse.size() || sparse[entityID] == INVALID_INDEX) {
			return;
		}
		uint32_t denseIndex = sparse[entityID];
		uint32_t denseLast = dense.size() - 1;
		uint32_t entitiesLast = entities[denseLast];
		dense[denseIndex] = dense[denseLast];
		entities[denseIndex] = entities[denseLast];
		sparse[entities[denseIndex]] = denseIndex;
		dense.pop_back();
		entities.pop_back();
	}

	const std::vector<uint32_t>& getEntities() const {
		return entities;
	}
};

inline const uint32_t PAGE_SIZE = 512;
struct Page {
    uint32_t data[PAGE_SIZE];
    Page() { std::fill(std::begin(data), std::end(data), UINT32_MAX); } // Required due to possible garbage data access
};

// PAGED SHOULD ONLY BE USED FOR INIT ENTITIES UNTIL WE HAVE VERSION CONTROL
template <typename Component>
class PagedSparseSet {
private:
	std::vector<std::unique_ptr<Page>> sparsePages;
	std::vector<Component> dense;
	std::vector<uint32_t> entities;

public:
	bool hasComponent(uint32_t entityID) const {
        uint32_t pageIndex = entityID / PAGE_SIZE;
        if (pageIndex >= sparsePages.size() || sparsePages[pageIndex] == nullptr) return false;
		uint32_t denseIndex = sparsePages[pageIndex]->data[entityID % PAGE_SIZE];
		return denseIndex != UINT32_MAX && denseIndex < dense.size() && entities[denseIndex] == entityID;
	}

	Component& getComponent(uint32_t entityID) {
        uint32_t pageIndex = entityID / PAGE_SIZE;
		uint32_t denseIndex = sparsePages[pageIndex]->data[entityID % PAGE_SIZE];
		return dense[denseIndex];
	}

	void add(uint32_t entityID, const Component& component) {
		uint32_t pageIndex = entityID / PAGE_SIZE;
		if (pageIndex + 1 >= sparsePages.size()) {
			sparsePages.resize(pageIndex + 1, nullptr);
		}
		if (sparsePages[pageIndex] == nullptr) {
			sparsePages[pageIndex] = std::make_unique<Page>;
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
		entities[denseIndex] = entities[denseLast];
        sparsePages[pageIndex]->data[entities[denseIndex] % PAGE_SIZE] = denseIndex;
		dense.pop_back();
		entities.pop_back();
	}

	const std::vector<uint32_t>& getEntities() const {
		return entities;
	}
};

