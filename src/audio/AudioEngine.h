#pragma once

#include "Sound.h"
#include "WavData.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>

class AudioEngine {
  public:
	AudioEngine();
	void close();
	void update(double dt);

	Sound createSound(std::string name);

	void updateListenerFrame(glm::mat4 viewMatrix);
	void updateListenerVel(float x, float y, float z);
	void updateSoundLoc(Sound sound, float x, float y, float z);
	void updateSoundVel(Sound sound, float x, float y, float z);

	static bool checkALErrors(std::string location);

  private:
	ALCdevice *device;
	ALCcontext *context;

	void loadSounds();

	std::map<std::string, WavData> sounds;
	std::vector<Sound> channels;

	struct ListenerData {
		glm::mat4 viewMatrix = glm::mat4(0);
		float velx = 0, vely = 0, velz = 0;
	};

	ListenerData listener;
};
