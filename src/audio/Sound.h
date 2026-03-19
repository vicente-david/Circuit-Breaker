#pragma once

#include <AL/al.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <string>

class Sound {
  public:
	Sound(){
	soundName="Default Constructor :(";}
	Sound(ALuint source, std::string name);
	std::string soundName;
	ALuint source;
	bool freed = false;

	glm::vec3 position = glm::vec3(0,0,0);
	glm::vec3 velocity = glm::vec3(0,0,0);

	void stop();
	void start();
	void setLooping(bool looping);
	// void updatePosition(float x, float y, float z);
	// void updateVelocity(float x, float y, float z);
};
