#pragma once
#include "asset_manager.h"
#include "camera.h"
#include "entity.h"
#include "sparse_set.h"
#include <cstdint>
#include <cstring>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

struct ComponentBlob {
    const std::type_info* typeInfo;
    size_t size;
    std::unique_ptr<uint8_t[]> data;

    template <typename T>
    ComponentBlob(const T& component);

    ComponentBlob(ComponentBlob&& other) noexcept;
    ComponentBlob(const ComponentBlob&) = delete;
    ComponentBlob& operator=(const ComponentBlob&) = delete;
};

template <typename T>
T& deserializeBlob(ComponentBlob& blob);

template <typename T>
ComponentBlob::ComponentBlob(const T& component) {
    typeInfo = &typeid(T);
    size = sizeof(T);
    data = std::make_unique<uint8_t[]>(sizeof(T));
    memcpy(data.get(), &component, size);
}

template <typename T>
T& deserializeBlob(ComponentBlob& blob) {
    if (*blob.typeInfo != typeid(T)) {
        throw std::runtime_error("Type mismatch!");
    }
    return *reinterpret_cast<T*>(blob.data.get());
}

struct MeshHandleStorage {
    uint32_t meshHandle;
};

struct MaterialHandleStorage {
    uint32_t materialHandle;
};

struct EntityTemplate {
    std::string name;
    std::vector<ComponentBlob> components;
};

class ECS {
public:
    ECS();

    void parseEntityTemplateFile();
    void parseSceneFile();
    void useEntityTemplate(const std::string& entityName);

    AssetManager assetManager;

    std::unordered_map<std::string, EntityTemplate> entityTemplates;
    std::vector<glm::vec3> pointLightPositions;

    uint32_t entityCount;

    // Currently there are no paged sparse sets as there are no sparse
    // components that are spaced far apart. i.e. greater than one entity has a
    // component and these entities differ largely in entity count.
    SparseSet<TransformComponent> transformSet;
    PagedSparseSet<MeshData> meshSet;
    SparseSet<MaterialData> materialSet;
    SparseSet<SkyboxTag> skyboxSet;
    SparseSet<InstancedComponent> instancedSet;
    SparseSet<PlayerInputWorldTag> inputWorldSet;
    SparseSet<PlayerInputTankTag> inputTankSet;
    SparseSet<PlayerInputNoClipTag> inputNoClipSet;
    SparseSet<VelocityComponent> velocitySet;
    SparseSet<SpeedComponent> speedSet;
    SparseSet<RotationSpeedComponent> rotationSpeedSet;
    SparseSet<PatrolComponent> patrolSet;
    SparseSet<CollisionComponent> collisionSet;
    SparseSet<RenderableTag> renderableSet;
    SparseSet<CameraComponent> cameraSet;
};
