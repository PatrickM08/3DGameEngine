#define _CRT_SECURE_NO_WARNINGS
#include "text.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <cstdio>
#include <cstring>
#include <iostream>

void parseFont(const char* path, Glyph* glyphs, uint16_t& glyphCount) {
    FILE* file = fopen(path, "rb");
    if (!file) { // TODO: MAKE THIS MORE ROBUST AND LOOK INTO STD::FROM_CHARS FOR BELOW
        std::cout << "FAILED TO LOAD FNT FILE." << std::endl;
    }
    char buffer[256];

    while (std::fgets(buffer, sizeof(buffer), file)) {
        if (std::strncmp(buffer, "char", 4) == 0 && glyphCount < MAX_GLYPHS) {
            Glyph g;
            int result = sscanf(buffer,
                                "char id=%d x=%d y=%d width=%d height=%d "
                                "xoffset=%d yoffset=%d xadvance=%d",
                                &g.id, &g.x, &g.y, &g.width, &g.height,
                                &g.xoffset, &g.yoffset, &g.xadvance);
            int index = g.id - 32;
            if (index >= 0 && index < MAX_GLYPHS) {
                glyphs[index] = g;
            }
            ++glyphCount;
        }
    }
    std::fclose(file);
}

uint32_t loadBitmapFont(const char* path, Glyph* glyphs, uint16_t glyphCount) {
    uint32_t textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RED;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        for (int i = 0; i < glyphCount; ++i) { // this is under the assumption our glyph buffer is full from the beginning. TODO: NEEDS TO BE CHANGED
            Glyph& glyph = glyphs[i];
            glyph.u0 = (float)glyph.x / width;
            glyph.v0 = (float)glyph.y / height;

            glyph.u1 = (float)(glyph.x + glyph.width) / width;
            glyph.v1 = (float)(glyph.y + glyph.height) / height;
        }

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void setupTextBuffers(uint32_t& textVAO, uint32_t& textVBO) {
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_TEXT_SIZE * 24 * sizeof(float), nullptr,
                 GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void renderTextSystem(TextBuffer& textBuffer, TextRenderData& textRenderData,
                      uint32_t screenWidth, uint32_t screenHeight) {
    if (textBuffer.size == 0) return;

    glUseProgram(textRenderData.textShaderID);
    glBindVertexArray(textRenderData.textVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textRenderData.bitmapFontTextureID);

    glUniform2f(0, (float)screenWidth, (float)screenHeight);

    int totalCharactersProcessed = 0;
    float* vertices = textRenderData.vertices;

    for (uint32_t i = 0; i < textBuffer.size; ++i) {
        TextEntry& entry = textBuffer.buffer[i];
        float currentX = entry.xPos;

        for (uint32_t j = 0; j < entry.textLength; ++j) {
            if (totalCharactersProcessed >= MAX_TEXT_SIZE) break;

            Glyph& glyph = textRenderData.glyphs[entry.text[j] - 32];
            int base = totalCharactersProcessed * 24;

            float left = currentX + (glyph.xoffset * entry.size);
            float top = entry.yPos - (glyph.yoffset * entry.size);
            float right = left + (glyph.width * entry.size);
            float bottom = top - (glyph.height * entry.size);

            // Triangle 1
            vertices[base + 0] = left;
            vertices[base + 1] = top;
            vertices[base + 2] = glyph.u0;
            vertices[base + 3] = glyph.v0;
            vertices[base + 4] = left;
            vertices[base + 5] = bottom;
            vertices[base + 6] = glyph.u0;
            vertices[base + 7] = glyph.v1;
            vertices[base + 8] = right;
            vertices[base + 9] = bottom;
            vertices[base + 10] = glyph.u1;
            vertices[base + 11] = glyph.v1;

            // Triangle 2
            vertices[base + 12] = left;
            vertices[base + 13] = top;
            vertices[base + 14] = glyph.u0;
            vertices[base + 15] = glyph.v0;
            vertices[base + 16] = right;
            vertices[base + 17] = bottom;
            vertices[base + 18] = glyph.u1;
            vertices[base + 19] = glyph.v1;
            vertices[base + 20] = right;
            vertices[base + 21] = top;
            vertices[base + 22] = glyph.u1;
            vertices[base + 23] = glyph.v0;

            currentX += glyph.xadvance * entry.size;
            totalCharactersProcessed++;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, textRenderData.textVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, totalCharactersProcessed * 24 * sizeof(float), vertices);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 6 * totalCharactersProcessed);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    textBuffer.size = 0;
}