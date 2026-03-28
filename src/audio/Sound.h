#pragma once

#include "PxRigidBody.h"
#include <AL/al.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <string>

class Sound {
  public:
	Sound() {
		soundName = "Default Constructor :(";
		freed = true;
	}
	Sound(ALuint source, std::string name);
	std::string soundName;
	ALuint source;
	bool freed = false;
	bool playing = false;
	bool do3D = true;

	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 velocity = glm::vec3(0, 0, 0);

	void stop();
	void pause();
	void start();
	void volume(float gain);
	void pitchMulti(float pitch);

	void setLooping(bool looping);
	void updateFromRbody(physx::PxRigidBody* rbody);
	// void updatePosition(float x, float y, float z);
	// void updateVelocity(float x, float y, float z);
};
