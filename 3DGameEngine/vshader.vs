#version 430 core
layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform SceneData{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition;
    int pointLightCount;
} sceneData;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{   
    gl_Position = sceneData.projectionMatrix * sceneData.viewMatrix * model * vec4(aPos, 1.0);
}