#include "Window.h"
#include <iostream>
#include <stdexcept>

Window::Window(uint32_t width, uint32_t height, const char* title)
    : width(width), height(height), title(title)
{
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) {
        auto* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(win));
        windowPtr->pushEvent(Event{ .type = EventType::WindowResize, .resize = {w, h} });
        });
    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xPos, double yPos) {
        auto* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(win));
        windowPtr->pushEvent(Event{ .type = EventType::MouseMove, .mouseMove = {(float)xPos, (float)yPos} });
        });
    glfwSetScrollCallback(window, [](GLFWwindow* win, double xOffset, double yOffset) {
        auto* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(win));
        windowPtr->pushEvent(Event{ .type = EventType::Scroll, .scroll = {(float)yOffset} });
        });
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }
    glViewport(0, 0, width, height);
}

Window::~Window() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void Window::pushEvent(const Event& event) {
    eventQueue.push(event);
}

bool Window::pollEvent(Event& event) {
    if (!eventQueue.empty()) {
        event = eventQueue.front();
        eventQueue.pop();
        return true;
    }
    return false;
}

void Window::processEvents() {
    glfwPollEvents();
}

GLFWwindow* Window::getGlfwWindowPtr() {
    return window;
}

/*
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
*/