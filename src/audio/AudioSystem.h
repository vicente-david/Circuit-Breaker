#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <string>

class AudioSystem{
	public:
		bool init();
		void close();
		bool checkALErrors(std::string location);



	private:
		ALCdevice* device;
		ALCcontext* context;
		ALuint source;
		ALuint buffer;

};
