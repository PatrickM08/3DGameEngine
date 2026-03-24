#include "editor.h"
#include "ecs.h"
#include "application.h"
#include "imgui.h"
#include "serialization.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void drawKeyBind(const char* label, uint16_t& keyIndex, ECS& scene, int slotID) {
    const char* keyName = glfwGetKeyName(keyIndex + 31, 0);
    char fallback[16];
    if (!keyName) {
        snprintf(fallback, sizeof(fallback), "key %d", keyIndex + 31);
        keyName = fallback;
    }

    ImGui::Text("%s: [%s]", label, keyName);
    ImGui::SameLine();

    char btnLabel[32];
    snprintf(btnLabel, sizeof(btnLabel), "Bind##%d", slotID);

    if (scene.awaitingBind == slotID) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "[ Press a key... ]");
        if (scene.lastPressedGLFWKey != -1) {
            keyIndex = (uint16_t)(scene.lastPressedGLFWKey - 31);
            scene.awaitingBind = -1;
            scene.lastPressedGLFWKey = -1;
        }
    } else {
        if (ImGui::Button(btnLabel)) {
            scene.lastPressedGLFWKey = -1;
            scene.awaitingBind = slotID;
        }
    }
}


void setGui(ECS& scene) {
    if (!scene.debugMode) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Once);
    ImGui::Begin("Editor");
    static float samples[500] = {};
    static int sampleIndex = 0;
    static float avgFPS = 0.0f;

    samples[sampleIndex % 500] = 1.0f / scene.deltaTime;
    sampleIndex++;

    if (sampleIndex % 500 == 0) {
        avgFPS = 0.0f;
        for (float s : samples)
            avgFPS += s;
        avgFPS /= 500.0f;
    }

    ImGui::Text("FPS: %.1f", avgFPS);
    ImGui::Text("Entities: %d", (int)scene.transformSet.entityCount);
    ImGui::Text("Visible Entities: %d", (int)scene.visibleEntityBuffer.size);
    ImGui::Text("Visible Lights: %d", (int)scene.visiblePointLightBuffer.size);
    ImGui::Separator();

    int& selectedEntity = scene.selectedEntity;
    if (ImGui::CollapsingHeader("Entities")) {
        for (uint32_t i = 0; i < scene.transformSet.entityCount; i++) {
            uint32_t entity = scene.transformSet.entities[i];
            char label[64];
            if (scene.nameSet.hasComponent(entity)) snprintf(label, sizeof(label), "Entity %s", scene.nameSet.getComponent(entity).name);
            else snprintf(label, sizeof(label), "Entity %d", entity);
            if (ImGui::Selectable(label, selectedEntity == (int)entity)) {
                selectedEntity = (int)entity;
            }
        }
    }

    // Material Buffer
    if (ImGui::CollapsingHeader("Materials")) {
        for (uint8_t i = 0; i < scene.materialBuffer.size; i++) {
            char label[32];
            snprintf(label, sizeof(label), "Material %d", i);
            if (ImGui::TreeNode(label)) {
                MaterialSSBOData& mat = scene.materialSSBODataBuffer.buffer[i];
                glm::vec3 colour = glm::vec3(mat.colourAndShine);
                if (ImGui::ColorEdit3("Colour", &colour.x)) {
                    mat.colourAndShine.x = colour.x;
                    mat.colourAndShine.y = colour.y;
                    mat.colourAndShine.z = colour.z;
                    updateMaterialColour(scene.materialBuffer.buffer[i],
                                        scene.materialSSBODataBuffer.buffer[scene.materialBuffer.buffer[i].materialSSBOIndex], colour, scene.materialSSBO);
                }
                ImGui::TreePop();
            }
        }
    }
    
    // Create Entity
    if (ImGui::Button("Create Blank Entity")) scene.transformSet.add(createEntity(scene), TransformComponent{});

    ImGui::InputText("File Name", scene.fileNameBuffer, sizeof(scene.fileNameBuffer));
    if (ImGui::Button("Save Scene")) {
        saveScene(scene, scene.fileNameBuffer);
    }
    if (ImGui::Button("Load Scene")) {
        loadScene(scene, scene.fileNameBuffer);
    }

    if (selectedEntity >= 0) {
        uint32_t e = (uint32_t)selectedEntity;
        ImGui::Separator();
        if (scene.nameSet.hasComponent(e)) ImGui::Text("Inspecting Entity %s", scene.nameSet.getComponent(e).name);
        else ImGui::Text("Inspecting Entity %d", e);

        // Name Component
        if (scene.nameSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Name")) {
                NameComponent& n = scene.nameSet.getComponent(e);
                ImGui::InputText("##Name", n.name, sizeof(n.name));
                if (ImGui::Button("Remove##Name")) scene.nameSet.remove(e);
            }
        } else {
            if (ImGui::Button("Add Name")) {
                scene.nameSet.add(e, NameComponent{});
            }
        }

        // Transform Component
        if (scene.transformSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Transform")) {
                TransformComponent& t = scene.transformSet.getComponent(e);
                ImGui::DragFloat3("Position", &t.position.x, 0.1f);
                ImGui::DragFloat3("Scale", &t.scale.x, 0.1f, 0.01f, 100.0f);
                glm::vec3 euler = glm::degrees(glm::eulerAngles(t.rotation));
                if (ImGui::DragFloat3("Rotation", &euler.x, 1.0f)) {
                    t.rotation = glm::quat(glm::radians(euler));
                }
                bool canRemove = !scene.dynamicSet.hasComponent(e) &&
                                 !scene.collisionSet.hasComponent(e) &&
                                 !scene.bulletSet.hasComponent(e);
                ImGui::BeginDisabled(!canRemove);
                if (ImGui::Button("Remove Transform")) scene.transformSet.remove(e);
                ImGui::EndDisabled();
                if (!canRemove) ImGui::TextDisabled("Remove Dynamic, Collision, Bullet first");
            }
        } else {
            if (ImGui::Button("Add Transform")) {
                scene.transformSet.add(e, TransformComponent{
                                              .rotation = glm::quat(1, 0, 0, 0),
                                              .position = glm::vec3(0),
                                              .scale = glm::vec3(1)});
            }
        }

        // Camera Component
        if (scene.cameraSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Camera")) {
                CameraComponent& c = scene.cameraSet.getComponent(e);
                ImGui::DragFloat3("Offset", &c.positionOffset.x, 0.1f);
                bool yawChanged = ImGui::DragFloat("Yaw", &c.yaw, 0.5f, -180.0f, 180.0f);
                bool pitchChanged = ImGui::DragFloat("Pitch", &c.pitch, 0.5f, -89.0f, 89.0f);
                if (yawChanged || pitchChanged) updateCameraVectors(c);
                int cameraType = (int)c.cameraType;
                const char* cameraTypes[] = {"Fixed", "MouseTurn"};
                if (ImGui::Combo("Camera Type", &cameraType, cameraTypes, 2)) {
                    c.cameraType = (CameraType)cameraType;
                }
                bool canRemove = scene.currentCamera != e;
                ImGui::BeginDisabled(!canRemove);
                if (ImGui::Button("Remove##Camera")) scene.cameraSet.remove(e);
                ImGui::EndDisabled();
                if (!canRemove) ImGui::TextDisabled("Set different current camera first.");
                if (ImGui::Button("Set##Camera")) scene.currentCamera = e;
            }
        } else {
            if (ImGui::Button("Add Camera")) {
                scene.cameraSet.add(e, CameraComponent{
                                           .positionOffset = glm::vec3(0.0f), 
                                           .position = glm::vec3(0.0f),
                                           .worldUp = glm::vec3(0, 1, 0),
                                           .yaw = -90.0f,
                                           .pitch = -60.0f,
                                           .cameraType = CameraType::MOUSETURN});
            }
        }

        // Health Component
        if (scene.healthSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Health")) {
                HealthComponent& h = scene.healthSet.getComponent(e);
                ImGui::SliderInt("HP", (int*)&h.health, 0, 10);
                if (ImGui::Button("Remove Health")) scene.healthSet.remove(e);
            }
        } else {
            if (ImGui::Button("Add Health")) {
                scene.healthSet.add(e, HealthComponent{3});
            }
        }

        // Velocity Component
        if (scene.velocitySet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Velocity")) {
                VelocityComponent& v = scene.velocitySet.getComponent(e);
                ImGui::DragFloat3("Velocity", &v.velocity.x, 0.1f);
                bool canRemove = !scene.dynamicSet.hasComponent(e) &&
                                 !scene.patrolSet.hasComponent(e) &&
                                 !scene.inputTankSet.hasComponent(e) &&
                                 !scene.inputWorldSet.hasComponent(e) &&
                                 !scene.inputNoClipSet.hasComponent(e);
                ImGui::BeginDisabled(!canRemove);
                if (ImGui::Button("Remove Velocity")) scene.velocitySet.remove(e);
                ImGui::EndDisabled();
                if (!canRemove) ImGui::TextDisabled("Remove Dynamic, Patrol, Input first");
            }
        } else {
            if (ImGui::Button("Add Velocity")) {
                scene.velocitySet.add(e, VelocityComponent{glm::vec3(0)});
            }
        }

        // Speed Component
        if (scene.speedSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Speed")) {
                SpeedComponent& s = scene.speedSet.getComponent(e);
                ImGui::DragFloat("##Speed", &s.speed, 0.1f, 0.0f, 100.0f);
                bool canRemove = !scene.patrolSet.hasComponent(e) &&
                                 !scene.inputTankSet.hasComponent(e) &&
                                 !scene.inputWorldSet.hasComponent(e) &&
                                 !scene.inputNoClipSet.hasComponent(e);
                ImGui::BeginDisabled(!canRemove);
                if (ImGui::Button("Remove Speed")) scene.speedSet.remove(e);
                ImGui::EndDisabled();
                if (!canRemove) ImGui::TextDisabled("Remove Patrol, Input first");
            }
        } else {
            if (ImGui::Button("Add Speed")) {
                scene.speedSet.add(e, SpeedComponent{5.0f});
            }
        }

        // Rotation Speed Component
        if (scene.rotationSpeedSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Rotation Speed")) {
                RotationSpeedComponent& r = scene.rotationSpeedSet.getComponent(e);
                ImGui::DragFloat("##Rotation Speed", &r.rotationSpeed, 1.0f, 0.0f, 360.0f);
                bool canRemove = !scene.inputTankSet.hasComponent(e);
                ImGui::BeginDisabled(!canRemove);
                if (ImGui::Button("Remove Rotation Speed")) scene.rotationSpeedSet.remove(e);
                ImGui::EndDisabled();
                if (!canRemove) ImGui::TextDisabled("Remove Tank Input first");
            }
        } else {
            if (ImGui::Button("Add Rotation Speed")) {
                scene.rotationSpeedSet.add(e, RotationSpeedComponent{100.0f});
            }
        }

        // Point Light Component
        if (scene.pointLightSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Point Light")) {
                PointLightComponent& l = scene.pointLightSet.getComponent(e);
                ImGui::ColorEdit3("Colour", &l.colour.x);
                ImGui::DragFloat("Intensity", &l.intensity, 0.1f, 0.0f, 10.0f);
                ImGui::DragFloat("Radius", &l.radius, 0.5f, 0.0f, 100.0f);
                if (ImGui::Button("Remove Point Light")) scene.pointLightSet.remove(e);
            }
        } else {
            if (ImGui::Button("Add Point Light")) {
                scene.pointLightSet.add(e, PointLightComponent{
                                               .colour = glm::vec3(1, 1, 1),
                                               .intensity = 1.0f,
                                               .radius = 10.0f});
            }
        }

        // Mesh Component
        if (scene.meshSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Mesh")) {
                MeshData& mesh = scene.meshSet.getComponent(e);
                int meshIndex = (int)mesh.handle;
                if (ImGui::SliderInt("Mesh Index", &meshIndex, 0, (int)scene.meshBuffer.size)) {
                    mesh.handle = (uint16_t)meshIndex;
                }
            }
        } else {
            if (ImGui::Button("Add Mesh")) {
                scene.meshSet.add(e, scene.meshBuffer.buffer[0]);
            }
        }

        // Material Component
        if (scene.materialSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Material")) {
                MaterialData& m = scene.materialSet.getComponent(e);
                int matIndex = (int)m.materialSSBOIndex;
                if (ImGui::SliderInt("Material Index", &matIndex, 0, (int)scene.materialSSBODataBuffer.size - 1)) {
                    m.materialSSBOIndex = (uint16_t)matIndex;
                }
            }
        } else {
            if (ImGui::Button("Add Material")) {
                scene.materialSet.add(e, scene.materialBuffer.buffer[0]);
            }
        }

        // Input Map Component
        if (scene.inputMapSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Input Map")) {
                InputMapComponent& im = scene.inputMapSet.getComponent(e);
                drawKeyBind("Forward", im.forwardIndex, scene, 0);
                drawKeyBind("Back", im.backIndex, scene, 1);
                drawKeyBind("Left", im.leftIndex, scene, 2);
                drawKeyBind("Right", im.rightIndex, scene, 3);
                drawKeyBind("Shoot", im.shootIndex, scene, 4);
                bool canRemove = !scene.inputTankSet.hasComponent(e) &&
                                 !scene.inputWorldSet.hasComponent(e) &&
                                 !scene.inputNoClipSet.hasComponent(e);
                ImGui::BeginDisabled(!canRemove);
                if (ImGui::Button("Remove##InputMap")) scene.inputMapSet.remove(e);
                ImGui::EndDisabled();
                if (!canRemove) ImGui::TextDisabled("Remove Input tags first");
            }
        }

        // Patrol Component
        bool patrolReqs = scene.velocitySet.hasComponent(e) && scene.speedSet.hasComponent(e);
        ImGui::BeginDisabled(!patrolReqs);
        if (scene.patrolSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Patrol")) {
                PatrolComponent& p = scene.patrolSet.getComponent(e);
                ImGui::DragFloat3("Direction", &p.direction.x, 0.1f, -1.0f, 1.0f);
                ImGui::DragFloat("Magnitude", &p.magnitude, 0.1f, 0.0f, 100.0f);
                if (ImGui::Button("Remove##Patrol")) scene.patrolSet.remove(e);
            }
        } else {
            glm::vec3& pendingDirection = scene.pendingDirection;
            float& pendingMagnitude = scene.pendingMagnitude;
            ImGui::DragFloat3("Direction##pending", &pendingDirection.x, 0.1f, -1.0f, 1.0f);
            ImGui::DragFloat("Magnitude##pending", &pendingMagnitude, 0.1f, 0.0f, 100.0f);
            if (ImGui::Button("Add Patrol")) {
                scene.patrolSet.add(e, PatrolComponent{
                                           .direction = pendingDirection,
                                           .magnitude = pendingMagnitude,
                                           .currentPatrolDistance = 0.0f});
            }
        }
        ImGui::EndDisabled();
        if (!patrolReqs) ImGui::TextDisabled("Patrol requires Velocity + Speed");

        // Collision Component
        bool collisionReqs = scene.transformSet.hasComponent(e);
        ImGui::BeginDisabled(!collisionReqs);
        if (scene.collisionSet.hasComponent(e)) {
            if (ImGui::CollapsingHeader("Collision")) {
                CollisionComponent& c = scene.collisionSet.getComponent(e);
                ImGui::DragFloat("Min X", &c.minX, 0.1f);
                ImGui::DragFloat("Max X", &c.maxX, 0.1f);
                ImGui::DragFloat("Min Y", &c.minY, 0.1f);
                ImGui::DragFloat("Max Y", &c.maxY, 0.1f);
                ImGui::DragFloat("Min Z", &c.minZ, 0.1f);
                ImGui::DragFloat("Max Z", &c.maxZ, 0.1f);
                bool canRemove = !scene.dynamicSet.hasComponent(e);
                ImGui::BeginDisabled(!canRemove);
                if (ImGui::Button("Remove Collision")) scene.collisionSet.remove(e);
                ImGui::EndDisabled();
                if (!canRemove) ImGui::TextDisabled("Remove Dynamic first");
            }
        } else {
            if (ImGui::Button("Add Collision")) {
                TransformComponent& t = scene.transformSet.getComponent(e);
                glm::vec3 half = t.scale * 0.5f;
                scene.collisionSet.add(e, CollisionComponent{
                                              .minX = -half.x, .maxX = half.x, .minY = -half.y, .maxY = half.y, .minZ = -half.z, .maxZ = half.z});
            }
        }
        ImGui::EndDisabled();
        if (!collisionReqs) ImGui::TextDisabled("Collision requires Transform");

        // Dynamic Component
        bool dynamicReqs = scene.transformSet.hasComponent(e) &&
                           scene.collisionSet.hasComponent(e) &&
                           scene.velocitySet.hasComponent(e);
        ImGui::BeginDisabled(!dynamicReqs);
        if (scene.dynamicSet.hasComponent(e)) {
            ImGui::Text("[ Dynamic ]");
            ImGui::SameLine();
            if (ImGui::Button("Remove##Dynamic")) scene.dynamicSet.remove(e);
        } else {
            if (ImGui::Button("Add Dynamic")) scene.dynamicSet.add(e, DynamicTag{});
        }
        ImGui::EndDisabled();
        if (!dynamicReqs) ImGui::TextDisabled("Dynamic requires Transform + Collision + Velocity");

        // Bullet Component
        bool bulletReqs = scene.transformSet.hasComponent(e) && scene.velocitySet.hasComponent(e);
        ImGui::BeginDisabled(!bulletReqs);
        if (scene.bulletSet.hasComponent(e)) {
            ImGui::Text("[ Bullet ]");
            ImGui::SameLine();
            if (ImGui::Button("Remove##Bullet")) scene.bulletSet.remove(e);
        } else {
            if (ImGui::Button("Add Bullet")) scene.bulletSet.add(e, BulletTag{});
        }
        ImGui::EndDisabled();
        if (!bulletReqs) ImGui::TextDisabled("Bullet requires Transform + Velocity");

        // Renderable Tag
        bool renderableReqs = scene.meshSet.hasComponent(e) && scene.materialSet.hasComponent(e);
        ImGui::BeginDisabled(!renderableReqs);
        if (scene.renderableSet.hasComponent(e)) {
            ImGui::Text("[ Renderable ]");
            ImGui::SameLine();
            if (ImGui::Button("Remove##Renderable")) scene.renderableSet.remove(e);
        } else {
            if (ImGui::Button("Add Renderable")) scene.renderableSet.add(e, RenderableTag{});
        }
        ImGui::EndDisabled();
        if (!renderableReqs) ImGui::TextDisabled("Renderable requires Mesh + Material");

        // Tank Input Tag
        bool tankReqs = scene.velocitySet.hasComponent(e) &&
                        scene.speedSet.hasComponent(e) &&
                        scene.transformSet.hasComponent(e) &&
                        scene.rotationSpeedSet.hasComponent(e) &&
                        scene.inputMapSet.hasComponent(e);
        ImGui::BeginDisabled(!tankReqs);
        if (scene.inputTankSet.hasComponent(e)) {
            ImGui::Text("[ Input: Tank ]");
            ImGui::SameLine();
            if (ImGui::Button("Remove##TankInput")) scene.inputTankSet.remove(e);
        } else {
            if (ImGui::Button("Add Tank Input")) scene.inputTankSet.add(e, PlayerInputTankTag{});
        }
        ImGui::EndDisabled();
        if (!tankReqs) ImGui::TextDisabled("Tank Input requires Velocity + Speed + Transform + RotationSpeed + InputMap");

        // World Input Tag
        bool worldReqs = scene.velocitySet.hasComponent(e) &&
                         scene.speedSet.hasComponent(e) &&
                         scene.inputMapSet.hasComponent(e);
        ImGui::BeginDisabled(!worldReqs);
        if (scene.inputWorldSet.hasComponent(e)) {
            ImGui::Text("[ Input: World ]");
            ImGui::SameLine();
            if (ImGui::Button("Remove##WorldInput")) scene.inputWorldSet.remove(e);
        } else {
            if (ImGui::Button("Add World Input")) scene.inputWorldSet.add(e, PlayerInputWorldTag{});
        }
        ImGui::EndDisabled();
        if (!worldReqs) ImGui::TextDisabled("World Input requires Velocity + Speed + InputMap");

        // No Clip Input Tag
        ImGui::BeginDisabled(!worldReqs); // same reqs as world
        if (scene.inputNoClipSet.hasComponent(e)) {
            ImGui::Text("[ Input: NoClip ]");
            ImGui::SameLine();
            if (ImGui::Button("Remove##NoClipInput")) scene.inputNoClipSet.remove(e);
        } else {
            if (ImGui::Button("Add NoClip Input")) scene.inputNoClipSet.add(e, PlayerInputNoClipTag{});
        }
        ImGui::EndDisabled();
        if (!worldReqs) ImGui::TextDisabled("NoClip Input requires Velocity + Speed + InputMap");
    }

    ImGui::End();
}

void drawGui(ECS& scene) {
    if (!scene.debugMode) return;
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}