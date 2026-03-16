#pragma once

struct ECS;
struct GLFWwindow;

int run(ECS& scene);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
