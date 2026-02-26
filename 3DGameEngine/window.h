#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdint>

GLFWwindow* createWindow(uint32_t width, uint32_t height, const char* title);
void destroyWindow(GLFWwindow* windowPtr);