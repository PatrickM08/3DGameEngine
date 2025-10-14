#pragma once

#include <unordered_map>
#include "shader_s.h"

struct Glyph {
    int id, x, y, width, height, xoffset, yoffset, xadvance;
    float u0, v0;
    float u1, v1;
};


class Text {
public:
    Text(const char* fontInfoFile, const char* fontTextureFile);
    void renderText(const char* text, const float xPos, const float yPos, const float size, const uint32_t screenWidth, const uint32_t screenHeight);
private:
    std::unordered_map<int, Glyph> parseFont(const char* fontInfofile);
    uint32_t loadBitmapFont(const char* fontTextureFile);
    void setupTextBuffers();
private:
    std::unordered_map<int, Glyph> glyphs;
    GLuint bitmapFontTexture;
    Shader textShader;
    GLuint textVAO;
    GLuint textVBO;
};
