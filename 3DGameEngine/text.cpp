#define _CRT_SECURE_NO_WARNINGS
#include <unordered_map>
#include "shader_s.h"
#include "stb_image.h"
#include <glad/glad.h>
#include "text.hpp"
#include <iostream>  
#include <fstream>   
#include <vector>    
#include <glm/glm.hpp>  
const int MAX_TEXT_LENGTH = 30;

Text::Text(const char* fontInfoFile, const char* fontTextureFile) 
    : glyphs(parseFont(fontInfoFile)),
    bitmapFontTexture(loadBitmapFont(fontTextureFile)),
    textShader("text_vertex_shader.vs", "text_fragment_shader.fs")
{
    setupTextBuffers();
}

void Text::renderText(const char* text, const float xPos, const float yPos, const float size, const uint32_t screenWidth, const uint32_t screenHeight) {
    textShader.use();
    glBindVertexArray(textVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmapFontTexture);

    textShader.setVec2Uniform("screenPosition", glm::vec2(xPos, yPos));
    textShader.setFloatUniform("scale", size);
    textShader.setVec2Uniform("screenSize", glm::vec2(screenWidth, screenHeight));

    int length = strlen(text);
    static float vertices[MAX_TEXT_LENGTH * 24];
    int elementsToUse = length * 24;
    float currentX = 0.0f;
    for (int i = 0; i < length; i++) {
        auto pair = glyphs.find(int(text[i]));
        if (pair == glyphs.end())
            continue;
        Glyph glyph = pair->second;

        int base = i * 24;

        float left = currentX + glyph.xoffset;
        float right = left + glyph.width;
        float bottom = -glyph.yoffset;
        float top = -glyph.yoffset + glyph.height;

        // Top-left vertex
        vertices[base + 0] = left;
        vertices[base + 1] = top;
        vertices[base + 2] = glyph.u0;
        vertices[base + 3] = glyph.v0;

        // Bottom-left vertex
        vertices[base + 4] = left;
        vertices[base + 5] = bottom;
        vertices[base + 6] = glyph.u0;
        vertices[base + 7] = glyph.v1;

        // Bottom-right vertex
        vertices[base + 8] = right;
        vertices[base + 9] = bottom;
        vertices[base + 10] = glyph.u1;
        vertices[base + 11] = glyph.v1;

        // Top-left vertex
        vertices[base + 12] = left;
        vertices[base + 13] = top;
        vertices[base + 14] = glyph.u0;
        vertices[base + 15] = glyph.v0;

        // Bottom-right vertex
        vertices[base + 16] = right;
        vertices[base + 17] = bottom;
        vertices[base + 18] = glyph.u1;
        vertices[base + 19] = glyph.v1;

        // Top-right vertex
        vertices[base + 20] = right;
        vertices[base + 21] = top;
        vertices[base + 22] = glyph.u1;
        vertices[base + 23] = glyph.v0;

        currentX += glyph.xadvance;
    }
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, elementsToUse * sizeof(float), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6 * length);
}

std::unordered_map<int, Glyph> Text::parseFont(const char* path) {
    std::unordered_map<int, Glyph> glyphs;
    std::ifstream fileFnt(path);
    if (!fileFnt.is_open()) {
        throw std::runtime_error("Error opening fnt file.");
    }
    std::string dataFnt;
    while (std::getline(fileFnt, dataFnt)) {
        if (dataFnt.substr(0, 4) == "char") {
            Glyph g;
            int result = sscanf(dataFnt.c_str(),
                "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                &g.id, &g.x, &g.y, &g.width, &g.height,
                &g.xoffset, &g.yoffset, &g.xadvance);
            glyphs.emplace( g.id, g );
        }
    }
    return glyphs;
}


uint32_t Text::loadBitmapFont(const char* path) {
    uint32_t textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        for (auto& [id, glyph] : glyphs) {
            glyph.u0 = (float)glyph.x / width;
            glyph.v0 = (float)glyph.y / height;

            glyph.u1 = (float)(glyph.x + glyph.width) / width;
            glyph.v1 = (float)(glyph.y + glyph.height) / height;
        }

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void Text::setupTextBuffers() {
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_TEXT_LENGTH * 24 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
