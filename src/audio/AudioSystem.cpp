#include "AudioSystem.h"
#include "Sound.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <cstddef>
#include <cstdio>
#include <stdbool.h>
#include <string>
#include <system_error>
#include <utility>

// reference:
// https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/
// https://ffainelli.github.io/openal-example/
bool AudioSystem::init() {

	// use the deafult audio device
	device = alcOpenDevice(NULL);

	if (!device) {
		fprintf(stderr, "Default audo device couldn't be opened :(\n");
		return false;
	}

	// create a al context (i have no ideas what this actually is)
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	checkALErrors("creating AL Context");

	// create a source to play audio (i think this is like a channel?)
	alGenSources((ALuint)1, &source);
	checkALErrors("source gen");
	alSourcei(source, AL_LOOPING, AL_FALSE);
	checkALErrors("set no looping");

	// create buffer (holds sound data)
	alGenBuffers((ALuint)1, &buffer);

	Sound test ;
	test.load("assets/sounds/b.wav");

	return true;
}

void AudioSystem::close() {
	alcDestroyContext(context);
	alcCloseDevice(device);
}

// checks for the last error thrown by openal
//
// returns true if no err, returns false if err, and prints to stderr
bool AudioSystem::checkALErrors(std::string location) {
	ALenum err = alGetError();

	if (err == AL_NO_ERROR) {
		return true;
	}
	switch (err) {

		fprintf(stderr, "OpenAl Error at location: %s\n", location.c_str());
	case AL_INVALID_NAME:
		fprintf(stderr,
				"AL_INVALID_NAME: invalid ID was passed to AL function.\n");
		break;

	case AL_INVALID_ENUM:
		fprintf(stderr,
				"AL_INVALID_ENUM: invalid enum was passed to AL function.\n");
		break;
	case AL_INVALID_VALUE:
		fprintf(stderr,
				"AL_INVALID_VALUE: invalid value was passed to AL function.\n");
		break;
	case AL_INVALID_OPERATION:
		fprintf(stderr,
				"AL_INVALID_OPERATION: requested operation is invalid.\n");
		break;
	case AL_OUT_OF_MEMORY:
		fprintf(stderr, "AL_OUT_OF_MEMORY: openAL ran out of memory!.\n");
		break;
	default:
		fprintf(stderr, "UNKNOWN AL ERROR: good luck :(\n");
		break;
	}
	return false;
}
