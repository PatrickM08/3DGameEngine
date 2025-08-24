

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 instanceTranslation;

out vec2 texCoords;
out vec3 fragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat3 normalMatrix;

void main()
{
    vec3 worldPos = vec3(model * vec4(aPos,1.0));
    fragPos = worldPos + instanceTranslation;
    normal = normalMatrix * aNormal;
    gl_Position = projection * view * vec4(fragPos, 1.0);
    texCoords = aTexCoord;
}

