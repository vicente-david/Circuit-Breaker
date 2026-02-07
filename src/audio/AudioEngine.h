#pragma once

// foreward declare engine so that sound can store a reference to this
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
class AudioEngine;
#include "Sound.h"
#include "WavData.h"
#include <al.h>
#include <alc.h>
#include <map>
#include <string>
#include <vector>

class AudioEngine {
  public:
	AudioEngine();
	void close();
	void update(double dt);

	Sound createSound(std::string name);
	// ALuint playSound(std::string name,physx::PxVec3 location, physx::PxVec3
	// velocity);

	void updateListnerFrame(glm::mat4 viewMatrix);
	void updateListnerVel(float x, float y, float z);
	void updateSoundLoc(Sound sound,float x, float y, float z);
	void updateSoundVel(Sound sound,float x, float y, float z);

	static bool checkALErrors(std::string location);

  private:
	ALCdevice *device;
	ALCcontext *context;

	void loadSounds();

	std::map<std::string, WavData> sounds;
	std::vector<ALuint> channels;

	struct ListnerData {
		glm::mat4 viewMatrix;
		float velx, vely, velz;
	};

	ListnerData listner;
};
