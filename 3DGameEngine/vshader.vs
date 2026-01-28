#version 430 core
#extension GL_ARB_bindless_texture : require
layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform SceneData {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition;
    int pointLightCount;
    samplerCube skyboxCubemap;
} sceneData;

uniform mat4 model;

out vec3 fragPos;

void main()
{   
    fragPos = vec3(model * vec4(aPos,1.0));
    gl_Position = sceneData.projectionMatrix * sceneData.viewMatrix * vec4(fragPos, 1.0);
}