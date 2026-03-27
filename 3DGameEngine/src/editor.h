#pragma once
#include <cstdint>

struct ECS;

void drawKeyBind(const char* label, uint16_t& keyIndex, ECS& scene, int slotID);
void setGui(ECS& scene);
void drawGui(ECS& scene);