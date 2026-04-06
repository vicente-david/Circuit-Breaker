#version 460 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 position;
layout(location = 2) in vec4 colour;

out vec4 particleColour;

uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform mat4 VP;


void main() {
	vec3 particleCenter = position.xyz;
	vec3 vertWorldPos = particleCenter + cameraRight * vertex.x * position.w + cameraUp * vertex.y * position.w;
	
	gl_Position = VP * vec4(vertWorldPos, 1.0);
	vec3 blendCol = mix(colour.xyz, vec3(1, 1, 1), 0.25);
	particleColour = vec4(blendCol, colour.w);
}