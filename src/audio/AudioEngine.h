#pragma once

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

	ALuint createSound(std::string name);
	ALuint playSound(std::string name);
	ALuint playSound(std::string name,float x, float y, float z);

	static bool checkALErrors(std::string location);

  private:
	ALCdevice *device;
	ALCcontext *context;
	float test = 0;

	void loadSounds();

	// std::map<std::string, ALuint> soundBuffs;
	std::map<std::string, WavData> sounds;
	std::vector<ALuint> channels;
};
