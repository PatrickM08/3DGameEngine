#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <queue>
#include <cstdint>

enum class EventType {
    WindowResize,
    MouseMove,
    Scroll
};

struct Event {
    EventType type;
    union {
        struct { int width, height; } resize;
        struct { float xPos, yPos; } mouseMove;
        struct { float yOffset; } scroll;
    };
};

class Window {
private:
    GLFWwindow* window;
    std::queue<Event> eventQueue;

public:
    uint32_t width;
    uint32_t height;
    const char* title;

    Window(uint32_t width, uint32_t height, const char* title);
    ~Window();

    void pushEvent(const Event& event);
    bool pollEvent(Event& event);
    void processEvents();
    GLFWwindow* getGlfwWindowPtr();
};

