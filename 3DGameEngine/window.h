#pragma once
#include <cstdint>

struct GLFWwindow;

struct WindowData {
    uint32_t width;
    uint32_t height;
    const char* title; // TODO: THIS MIGHT BE BETTER AS A BUFFER CHECK LATER.
    GLFWwindow* windowPtr;
};

GLFWwindow* createWindow(uint32_t width, uint32_t height, const char* title);
void destroyWindow(GLFWwindow* windowPtr);