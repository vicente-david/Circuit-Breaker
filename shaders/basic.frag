#version 460 core

in vec3 fragColor;
in vec2 texCoord;

uniform sampler2D inTex;

out vec4 color;

void main() {
	color = texture(inTex, texCoord);
}