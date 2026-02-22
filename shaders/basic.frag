#version 460 core

in vec2 texCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D inTex;
uniform bool hasTex; // if no texture bound, render with default color

out vec4 color;

void main() {
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(vec3(0.3, 1.0, 1.0)); // directional light
	vec3 lightCol = vec3(1.0, 1.0, 1.0);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightCol;

	float ambientStr = 0.5;
	vec3 ambient = ambientStr * lightCol;

	vec4 objCol;
	if (hasTex)
		objCol = texture(inTex, texCoord);
	else
		objCol = vec4(0.5, 0.5, 0.5, 1.0);

	color = vec4(ambient + diffuse, 1.0) * objCol;
}