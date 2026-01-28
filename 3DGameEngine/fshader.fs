#version 430 core
#extension GL_ARB_bindless_texture : require
out vec4 FragColor;
in vec3 fragPos;

layout (std140, binding = 0) uniform SceneData {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition;
    int pointLightCount;
    samplerCube skyboxCubemap;
} sceneData;

struct PointLight{
    vec4 colourAndIntensity;
    vec4 positionAndRadius;
};

layout(std430, binding = 0) readonly buffer LightBuffer {
    PointLight pointLights[];
};

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 colour); 

void main()
{
    vec3 norm = normalize(vec3(0, 1, 0)); // Hardcoded for floor
    vec3 viewDir = normalize(sceneData.cameraPosition - fragPos);
    vec3 objectColour = vec3(0.5, 0.5, 0.5);
    vec3 result = objectColour * 0.4; 

    for (int i = 0; i < sceneData.pointLightCount; i++) {
        result += CalcPointLight(pointLights[i], norm, fragPos, viewDir, objectColour);
    }

    FragColor = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 colour)
{
    vec3 lightPos = light.positionAndRadius.xyz;
    float radius = light.positionAndRadius.w;
    vec3 lightCol = light.colourAndIntensity.xyz * light.colourAndIntensity.w;

    vec3 lightDir = normalize(lightPos - fragPos);
    float distance = length(lightPos - fragPos);

    // 2. STAGE 1: Check Radius
    if (distance > radius) return vec3(0.0);

    // 3. STAGE 2: Attenuation
    float ratio = distance / radius;
    float attenuation = clamp(1.0 - ratio * ratio, 0.0, 1.0);
    attenuation *= attenuation; // Smooth falloff

    // 4. STAGE 3: Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // 5. STAGE 4: Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 0.0);
    vec3 diffuse = lightCol * diff * colour;
    vec3 specular = lightCol * spec; // Specular usually doesn't take object colour

    return (diffuse * specular) * attenuation;
}