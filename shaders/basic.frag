#version 460 core

in vec2 texCoord;
in vec3 Normal;
in vec3 FragPos;
//in vec4 FragPosLightSpace;

uniform sampler2D inTex;
uniform sampler2DArray shadowMap;
uniform bool hasTex; // if no texture bound, render with default color
uniform float mixAmt; // for colouring effects, amount of color to add
uniform vec4 mixColour; // mixing colour

uniform float farPlane;
uniform mat4 view;
layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};
uniform float cascadePlaneDistances[16];
uniform int cascadeCount;   // number of frusta - 1

out vec4 color;

float shadowCalc(vec3 fragPosWorld, float bias) {

	// cascade layer
	vec4 fragPosView = view * vec4(fragPosWorld, 1.0);
	float depthVal = abs(fragPosView.z);

	int layer = -1;
	for (int i=0; i < cascadeCount; ++i){
		if (depthVal < cascadePlaneDistances[i]) {
			layer = i;
			break;
		}
	}
	if (layer == -1) {
		layer = cascadeCount;
	}

	vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorld, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // Perspective divide
	
	projCoords = projCoords * 0.5 + 0.5; // transform to range [0,1]

	float currentDepth = projCoords.z;
	
	// bias
	const float biasMod = 0.5f;
	if (layer == cascadeCount) {
		bias *= 1 / (farPlane * biasMod);
	}
	else {
		bias *= 1 / (cascadePlaneDistances[layer] * biasMod);
	}

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0)); // size of a single texel of the texture map
	for (int x = -2; x <= 2; ++x){
		for (int y = -2; y <= 2; ++y){
			float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r; // use the texel as offset to sample at different depth values
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0; // check if fragment in shadow
		}
	}
	shadow /= 25.0; // average results
	
	//shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0; // check if fragment in shadow
	if(projCoords.z > 1.0) {
		shadow = 0.0;
	}

	return shadow;
}


void main() {
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(vec3(0.0, 1.0, 0.1)); // directional light
	vec3 lightCol = vec3(1.0, 1.0, 1.0);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightCol;

	float ambientStr = 0.55;
	vec3 ambient = ambientStr * lightCol;

	vec4 objCol;
	if (hasTex)
		objCol = texture(inTex, texCoord);
	else
		objCol = vec4(0.5, 0.5, 0.5, 1.0);

	objCol = mix(objCol, mixColour, mixAmt);

	float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005); // For reducing shadow acne
	
	float shadow = shadowCalc(FragPos, bias);
	color = vec4(ambient + (1.0 - shadow) * diffuse, 1.0) * objCol;
}