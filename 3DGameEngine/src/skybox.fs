#version 430 core
#extension GL_ARB_bindless_texture : require
out vec4 FragColor;

in vec3 TexCoords;

layout (std140, binding = 0) uniform SceneData {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition;
    int pointLightCount;
    samplerCube skyboxCubemap;
} sceneData;

void main()
{    
    FragColor = texture(sceneData.skyboxCubemap, TexCoords);
}

