#version 460 core

in vec4 particleColour;
out vec4 colour;

void main(){
	colour = particleColour;
}