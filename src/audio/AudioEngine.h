#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <map>
#include <string>
#include <vector>


class AudioEngine{
	public:
		AudioEngine();
		void close();
		void update(double dt);

		ALuint playSound(std::string name);



	private:
		ALCdevice* device;
		ALCcontext* context;
		ALuint source;
		float test = 0;


		void loadSounds();
		bool checkALErrors(std::string location);

		std::map<std::string, ALuint> soundBuffs;
		std::vector<ALuint> channels;


};
