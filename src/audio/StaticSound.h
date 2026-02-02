#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <map>
#include <string>


class StaticAudio{
	public:
		bool init();
		void close();
		void playSound(std::string name);


	private:
		ALCdevice* device;
		ALCcontext* context;
		ALuint source;
		// ALuint buffer;

		void loadSounds();
		bool checkALErrors(std::string location);

		std::map<std::string, ALuint> buffers;

};
