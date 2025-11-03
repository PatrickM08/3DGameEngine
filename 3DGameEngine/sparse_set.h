#pragma once
#include <vector>
#include <cstdint>


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