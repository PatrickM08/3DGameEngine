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

class ECS {
public:
    ECS();

    AssetManager assetManager;

    static constexpr uint32_t MAX_ENTITIES = 2000000;
    uint32_t entityCount;

    // Currently there are no paged sparse sets as there are no sparse
    // components that are spaced far apart. i.e. greater than one entity has a
    // component and these entities differ largely in entity count.
    // 
    // TODO: NEED TO RESERVE MEMORY ONCE WE GET DYNAMIC ENTITY CREATION - THIS SHOULD BE AN ARENA FOR ALL COMPONENTS
    // TODO: SPARSE SETS SHOULD BE MADE CLEANER
    SparseSet<TransformComponent> transformSet;
    SparseSet<MeshData> meshSet;
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
    SparseSet<PointLightComponent> pointLightSet;

    std::vector<uint32_t> visibleEntities;
    std::vector<PackedLightData> visiblePointLights;
};

void init(ECS& scene);