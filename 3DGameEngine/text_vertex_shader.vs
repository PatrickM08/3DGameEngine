#version 430 core
layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexCoords;

layout(location = 0) uniform vec2 screenSize;

out vec2 TexCoords;

void main() 
{
    TexCoords = aTexCoords;

    gl_Position = vec4((aPos.x / screenSize.x) * 2.0 - 1.0, 
                       (aPos.y / screenSize.y) * 2.0 - 1.0, 
                       0.0, 1.0);
}