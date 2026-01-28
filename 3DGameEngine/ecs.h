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

struct SceneUBOData {
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 cameraPosition;
    uint32_t pointLightCount;
    uint64_t skyboxCubemapHandle;
};

struct SkyboxData {
    uint64_t cubemapHandle;
    uint32_t shaderID;
    uint32_t meshVAO;
};

GLuint createLightSSBO();
void performLightCulling(const SparseSet<PointLightComponent>& pointLightEntities,
                         const SparseSet<TransformComponent>& transformSet,
                         std::vector<PackedLightData>& visiblePointLights,
                         const glm::vec4* frustumPlanes);
void uploadLightSSBO(const GLuint lightSSBO, const std::vector<PackedLightData>& visiblePointLights);

GLuint createSceneUBO();
void updateSceneData(SceneUBOData& sceneData, const CameraComponent& camera, 
                    const std::vector<PackedLightData>& visiblePointLights, const SkyboxData& skyboxData);
void uploadSceneUBO(const GLuint sceneUBO, const SceneUBOData sceneData);

void performFrustumCulling(const std::vector<uint32_t>& renderableEntities,
                           const SparseSet<TransformComponent>& transformSet,
                           const SparseSet<MeshData>& meshSet,
                           std::vector<uint32_t>& visibleEntities,
                           const glm::vec4* frustumPlanes);

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
    GLuint lightSSBO;
    SkyboxData skyboxData;
    GLuint sceneUBO;
    SceneUBOData sceneData;
};

void init(ECS& scene);