#version 430 core
#extension GL_ARB_bindless_texture : require
out vec4 FragColor;
in vec2 texCoords;
in vec3 fragPos;
in vec3 normal;

layout (std140, binding = 0) uniform SceneData {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    // vec3 and int = 16 bytes - be careful of alignment if you want to change
    vec3 cameraPosition;
    int pointLightCount;
} sceneData;

struct PointLight {
    vec4 colourAndIntensity;
    vec4 positionAndRadius;
};

layout(std430, binding = 0) readonly buffer LightBuffer {
    PointLight pointLights[];
};

struct Material {
    sampler2D diffuseTexture;
    sampler2D specularTexture;
    float shininess;
};


layout(std430, binding = 1) readonly buffer MaterialBuffer {
    Material materials[];
};

layout (location = 2) uniform int materialIndex;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 textureColour, vec3 specularTextureColour, float shininess); 

void main()
{
    Material material = materials[materialIndex];
    vec3 viewDir = normalize(sceneData.cameraPosition - fragPos);
    vec3 result = vec3(0.0);
    vec3 textureColour = vec3(texture(material.diffuseTexture, texCoords));
    vec3 specularTextureColour = vec3(texture(material.specularTexture, texCoords));
    vec3 norm = normalize(normal);
    for (int i = 0; i < sceneData.pointLightCount; i++){
        result += CalcPointLight(pointLights[i], norm, fragPos, viewDir, textureColour, specularTextureColour, material.shininess);
    }
    FragColor = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 textureColour, vec3 specularTextureColour, float shininess)
{
    vec3 lightPosition = light.positionAndRadius.xyz;
    float lightRadius = light.positionAndRadius.w;
    vec3 lightColourAndIntensity = light.colourAndIntensity.xyz * light.colourAndIntensity.w;
    vec3 lightDir = normalize(lightPosition - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation - PROBLEMS HERE FIGURE OUT - THE AMBIENT SHOULD PROBABLY BE WITHOUT ATTENUTATION ITS WHAT IT IS REGARDLESS
    float distance = length(lightPosition - fragPos);
    float ratio = clamp(distance / lightRadius, 0.0, 1.0);
    float attenuation = (1.0 - ratio * ratio);
    attenuation *= attenuation;
    // combine results
    vec3 ambient  = lightColourAndIntensity * textureColour;
    vec3 diffuse  = lightColourAndIntensity * diff * textureColour;
    vec3 specular = lightColourAndIntensity * spec * specularTextureColour;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}