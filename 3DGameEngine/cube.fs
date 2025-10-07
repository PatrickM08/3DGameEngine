#version 330 core
out vec4 FragColor;
in vec2 texCoords;
in vec3 fragPos;
in vec3 normal;

struct PointLight{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float shininess;
uniform PointLight pointLights[32];
uniform int numberOfPointLights;
uniform vec3 cameraPos;

void main()
{
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 result = vec3(0.0);
    for (int i = 0; i < numberOfPointLights; i++){
        result += CalcPointLight(pointLights[i], normal, fragPos, viewDir);
    }
    FragColor = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(texture0, texCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture0, texCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture1, texCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}