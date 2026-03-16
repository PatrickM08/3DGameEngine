#pragma once
#include <cstdint>

// TODO: IM NOT SURE I LIKE THIS
static constexpr uint8_t MAX_GLYPHS = 96;
static constexpr uint8_t MAX_TEXT_LENGTH = 64;
static constexpr uint16_t MAX_TEXT_SIZE = 1024; // Max text size * max text length = text buffer capacity - should improve.

struct TextEntry {
    char text[MAX_TEXT_LENGTH];
    uint8_t textLength;
    float xPos;
    float yPos;
    float size;
};

struct TextBuffer {
    uint8_t size;
    static constexpr uint8_t capacity = 255;
    TextEntry buffer[capacity];
};

struct Glyph {
    int id, x, y, width, height, xoffset, yoffset, xadvance;
    float u0, v0;
    float u1, v1;
};

struct TextRenderData {
    Glyph glyphs[MAX_GLYPHS];
    float vertices[MAX_TEXT_SIZE * 24];
    uint32_t textShaderID = 0;
    uint32_t textVAO = 0;
    uint32_t textVBO = 0;
    uint32_t bitmapFontTextureID = 0;
    uint16_t glyphCount = 0;
};

void renderTextSystem(TextBuffer& textBuffer, TextRenderData& textRenderData,
                      uint32_t screenWidth, uint32_t screenHeight);
void parseFont(const char* path, Glyph* glyphs, uint16_t& glyphCount);
uint32_t loadBitmapFont(const char* path, Glyph* glyphs, uint16_t glyphCount);
void setupTextBuffers(uint32_t& textVAO, uint32_t& textVBO);
