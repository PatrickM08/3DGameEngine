#version 430 core
#extension GL_ARB_bindless_texture : require
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (std140, binding = 0) uniform SceneData {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition;
    int pointLightCount;
    samplerCube skyboxCubemap;
} sceneData;


void main()
{
    TexCoords = aPos;
    vec4 pos = sceneData.projectionMatrix * mat4(mat3(sceneData.viewMatrix)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  

