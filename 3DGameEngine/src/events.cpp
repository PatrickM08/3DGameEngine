#include "events.h"
#include "window.h"
#include "camera.h"
#include "render_system.h"
#include "imgui.h"
#include <glad/glad.h>

bool eventQueueEmpty(EventQueue& queue) {
    return queue.front == queue.back;
}

// There is one index always left empty between the back and the front to distinguish between an empty queue and a full queue.
void pushEvent(EventQueue& queue, const Event& event) {
    uint8_t next = (queue.back + 1) & 127;
    if (next != queue.front) {
        queue.ringBuffer[queue.back] = event;
        queue.back = next;
    }
}

Event popEvent(EventQueue& queue) {
    uint8_t front = queue.front;
    queue.front = (queue.front + 1) & 127;
    return queue.ringBuffer[front];
}

bool pollEvent(EventQueue& eventQueue, Event& event) {
    if (!eventQueueEmpty(eventQueue)) {
        event = popEvent(eventQueue);
        return true;
    }
    return false;
}

void handleWindowEvent(const Event& event, WindowData& window, Framebuffer& framebuffer, CameraComponent& camera,
                       MouseData& mouseData) {

    ImGuiIO& io = ImGui::GetIO();

    switch (event.type) {
    case EventType::WindowResize: {
        glViewport(0, 0, event.resize.width, event.resize.height);
        window.width = event.resize.width;
        window.height = event.resize.height;
        glDeleteFramebuffers(1, &framebuffer.buffer);
        glDeleteTextures(1, &framebuffer.textureAttachment);
        glDeleteRenderbuffers(1, &framebuffer.renderBufferObject);
        framebuffer = createFrameBuffer(framebuffer.shaderID, window.width, window.height);
        updateProjectionMatrix(camera, window.width, window.height);
        break;
    }

    case EventType::MouseMove: {
        if (!mouseData.hasBeenRecorded) {
            mouseData.lastCursorX = event.mouseMove.xPos;
            mouseData.lastCursorY = event.mouseMove.yPos;
            mouseData.hasBeenRecorded = true;
        }
        mouseData.frameOffsetX += event.mouseMove.xPos - mouseData.lastCursorX;
        mouseData.frameOffsetY += mouseData.lastCursorY - event.mouseMove.yPos;

        mouseData.lastCursorX = event.mouseMove.xPos;
        mouseData.lastCursorY = event.mouseMove.yPos;

        break;
    }
    case EventType::Scroll: {
        processMouseScroll(camera, event.scroll.yOffset);
        break;
    }
    }
}