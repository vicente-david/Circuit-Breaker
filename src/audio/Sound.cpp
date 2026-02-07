
#include "Sound.h"
#include "AudioEngine.h"
#include <al.h>
#include <string>

Sound::Sound(ALuint source, std::string name) {
	this->source = source;
	soundName = name;
}

void Sound::start() {
	alSourcePlay(source);
	AudioEngine::checkALErrors("playing " + soundName);
}

// this should only be used on looping, or still playing sounds
// if this is called on an already stopped sound, it may stop a
// different sound that has traken over its channel
void Sound::stop() {
	alSourceStop(source);
	AudioEngine::checkALErrors("playing " + soundName);
}

void Sound::setLooping(bool loop) {
	if (loop) {
		alSourcei(source, AL_LOOPING, AL_TRUE);
		AudioEngine::checkALErrors("setting yes looping");
	} else {
		alSourcei(source, AL_LOOPING, AL_FALSE);
		AudioEngine::checkALErrors("setting no looping");
	}
}
