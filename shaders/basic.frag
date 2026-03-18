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
	
	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // size of a single texel of the texture map
	for (int x = -1; x <= 1; ++x){
		for (int y = -1; y <= 1; ++y){
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; // use the texel as offset to sample at different depth values
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0; // check if fragment in shadow
		}
	}
	shadow /= 9.0; // average results
	
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

	float ambientStr = 0.5;
	vec3 ambient = ambientStr * lightCol;

	vec4 objCol;
	if (hasTex)
		objCol = texture(inTex, texCoord);
	else
		objCol = vec4(0.5, 0.5, 0.5, 1.0);

	float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005); // For reducing shadow acne

	float shadow = shadowCalc(FragPosLightSpace, bias);
	color = vec4(ambient + (1.0 - shadow) * diffuse, 1.0) * objCol;
}