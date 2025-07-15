#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D textTexture;

void main()
{
    float alpha = texture(textTexture, TexCoords).r;
    FragColor = vec4(1.0,1.0,1.0,alpha);
}