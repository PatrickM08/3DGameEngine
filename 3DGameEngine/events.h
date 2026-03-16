#pragma once
#include <cstdint>

struct WindowData;
struct Framebuffer;
struct CameraComponent;

enum class EventType {
    WindowResize,
    MouseMove,
    Scroll
};

struct Event {
    EventType type;
    union {
        struct {
            int width, height;
        } resize;
        struct {
            float xPos, yPos;
        } mouseMove;
        struct {
            float yOffset;
        } scroll;
    };
};

// TODO: NEED TO FIND HOW MUCH SPACE I ACTUALLY NEED FOR THIS.

struct EventQueue {
    Event ringBuffer[128];
    uint8_t front = 0;
    uint8_t back = 0;
};

struct MouseData {
    float lastCursorX = 0.0f;
    float lastCursorY = 0.0f;
    float frameOffsetX = 0.0f;
    float frameOffsetY = 0.0f;
    bool hasBeenRecorded = false;
};

bool eventQueueEmpty(EventQueue& queue);

// There is one index always left empty between the back and the front to distinguish between an empty queue and a full queue.
void pushEvent(EventQueue& queue, const Event& event);

Event popEvent(EventQueue& queue);

bool pollEvent(EventQueue& eventQueue, Event& event);

void handleWindowEvent(const Event& event, WindowData& window, Framebuffer& framebuffer, CameraComponent& camera,
                       MouseData& mouseData);