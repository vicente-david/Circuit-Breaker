#pragma once

#include <al.h>
#include <string>

class Sound {
  public:
	Sound(ALuint source, std::string name);
	std::string soundName;
	ALuint source;

	void stop();
	void start();
	void setLooping(bool looping);
	// void updatePosition(float x, float y, float z);
	// void updateVelocity(float x, float y, float z);

};
