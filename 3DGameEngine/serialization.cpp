#include "serialization.h"
#include "ecs.h"

void saveScene(ECS& scene, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) return;

    fwrite(&scene.entityCount, sizeof(uint32_t), 1, f);

    writeSet(f, scene.transformSet);
    writeSet(f, scene.velocitySet);
    writeSet(f, scene.speedSet);
    writeSet(f, scene.rotationSpeedSet);
    writeSet(f, scene.healthSet);
    writeSet(f, scene.collisionSet);
    writeSet(f, scene.patrolSet);
    writeSet(f, scene.pointLightSet);
    writeSet(f, scene.materialSet);
    writeSet(f, scene.meshSet);
    writeSet(f, scene.cameraSet);
    writeSet(f, scene.inputMapSet);
    writeSet(f, scene.renderableSet);
    writeSet(f, scene.dynamicSet);
    writeSet(f, scene.bulletSet);
    writeSet(f, scene.inputWorldSet);
    writeSet(f, scene.inputTankSet);
    writeSet(f, scene.inputNoClipSet);
    writeSet(f, scene.nameSet);

    fclose(f);
}

void loadScene(ECS& scene, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return;

    fread(&scene.entityCount, sizeof(uint32_t), 1, f);

    readSet(f, scene.transformSet);
    readSet(f, scene.velocitySet);
    readSet(f, scene.speedSet);
    readSet(f, scene.rotationSpeedSet);
    readSet(f, scene.healthSet);
    readSet(f, scene.collisionSet);
    readSet(f, scene.patrolSet);
    readSet(f, scene.pointLightSet);
    readSet(f, scene.materialSet);
    readSet(f, scene.meshSet);
    readSet(f, scene.cameraSet);
    readSet(f, scene.inputMapSet);
    readSet(f, scene.renderableSet);
    readSet(f, scene.dynamicSet);
    readSet(f, scene.bulletSet);
    readSet(f, scene.inputWorldSet);
    readSet(f, scene.inputTankSet);
    readSet(f, scene.inputNoClipSet);
    readSet(f, scene.nameSet);

    fclose(f);
}