#version 430 core
out vec4 FragColor;

in vec3 TexCoords;

layout (std140, binding = 0) uniform SceneData{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition;
    int pointLightCount;
} sceneData;

uniform samplerCube texture0;

void main()
{    
    FragColor = texture(texture0, TexCoords);
}

