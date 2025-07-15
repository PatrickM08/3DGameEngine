// glyphs
struct Glyph {
    int id, x, y, width, height, xoffset, yoffset, xadvance;
    float u0, v0;
    float u1, v1;
};

std::unordered_map<int, Glyph> parseFont(const char* path);
int loadBitmapFont(const char* path, std::unordered_map<int, Glyph>& glyphs);
void setupTextBuffers(unsigned int& textVBO, unsigned int& textVAO);
void renderText(const char* text, float x, float y, float size, unsigned int& textVBO, Shader& textShader,const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, const std::unordered_map<int, Glyph>& glyphs);