#version 460 core

in vec2 texCoord;

uniform sampler2D inTex;
uniform bool hasTex; // if no texture bound, render with default color

out vec4 color;

void main() {
	color = texture(inTex, texCoord);
}