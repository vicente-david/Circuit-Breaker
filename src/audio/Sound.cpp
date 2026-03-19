
#include "Sound.h"
#include "AudioEngine.h"
#include <AL/al.h>
#include <string>

Sound::Sound(ALuint source, std::string name) {
	this->source = source;
	soundName = name;
	if (source == -1) {
		freed = true;
	}
}

// starts the sound so it will start playing
void Sound::start() {
	if (freed) {
		return;
	}
	playing = true;
	alSourcePlay(source);
	AudioEngine::checkALErrors("playing " + soundName);
}

void Sound::stop() {
	if (freed) {
		return;
	}
	playing = false;
	alSourceStop(source);
	AudioEngine::checkALErrors("stopping " + soundName);
}

void Sound::setLooping(bool loop) {
	if (freed) {
		return;
	}
	if (loop) {
		alSourcei(source, AL_LOOPING, AL_TRUE);
		AudioEngine::checkALErrors("setting yes looping");
	} else {
		alSourcei(source, AL_LOOPING, AL_FALSE);
		AudioEngine::checkALErrors("setting no looping");
	}
}
