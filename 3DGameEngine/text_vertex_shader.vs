#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform vec2 screenPosition;
uniform float scale;
uniform vec2 screenSize;

out vec2 TexCoords;

void main() 
{
	vec2 scaledScreenPosition = (scale * aPos) + screenPosition;
	
	TexCoords = aTexCoords;
	gl_Position = vec4((scaledScreenPosition.x/screenSize.x)*2-1,(scaledScreenPosition.y/screenSize.y)*2-1, 0.0, 1.0);
}