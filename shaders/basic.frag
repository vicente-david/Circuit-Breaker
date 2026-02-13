#version 460 core

in vec2 texCoord;

uniform sampler2D inTex;
uniform bool hasTex; // if no texture bound, render with default color

out vec4 color;

void main() {
	if (hasTex)
		color = texture(inTex, texCoord);
	else
		color = vec4(0.5, 0.5, 0.5, 1.0);
}