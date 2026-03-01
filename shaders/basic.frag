#version 460 core

in vec2 texCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform sampler2D inTex;
uniform sampler2D shadowMap;
uniform bool hasTex; // if no texture bound, render with default color

out vec4 color;

float shadowCalc(vec4 fragPosLightSpace, float bias) {
	
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // Perspective divide
	
	projCoords = projCoords * 0.5 + 0.5; // transform to range [0,1]

	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0; // check if fragment in shadow

	return shadow;
}


void main() {
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(vec3(0.3, 3.0, 1.0)); // directional light
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

	float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
	float shadow = shadowCalc(FragPosLightSpace, bias);
	color = vec4(ambient + (1.0 - shadow) * diffuse, 1.0) * objCol;
}