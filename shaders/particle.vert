#version 460 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 position;
layout(location = 2) in vec4 colour;

uniform vec3 cameraUp;
uniform vec3 cameraRight;

out vec4 particleColour;

void main() {
	vec3 particleCenter = {position.x, position.y, position.z};
	vec3 vertWorldPos = particleCenter + cameraRight * vertex.x * position.w + cameraUp * vertex.y * position.w;
	
	particleColour = colour;
	gl_Position = vec4(vertWorldPos, 1.0);

}