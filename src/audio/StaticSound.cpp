#include "StaticSound.h"
#include "WavData.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <cstddef>
#include <cstdio>
#include <stdbool.h>
#include <string>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>


// reference:
// https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/
// https://ffainelli.github.io/openal-example/
//
// This class is a global singleton, play sound from anywhere type thing. (good for UI/music and such)
// a proper audio system for doing dopler/volume based on entity locations will probably be a seperate thing
bool StaticAudio::init() {

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

	loadSounds();

	alSourcei(source, AL_BUFFER, buffers["full"]);
	checkALErrors("buffer2");
	alSourcePlay(source);
	checkALErrors("play");

	return true;
}
void StaticAudio::loadSounds() {

	std::vector<WavData> files;
	files.push_back(WavData("hiya", "assets/sounds/hiya.wav"));
	files.push_back(WavData("full", "assets/sounds/aaa.wav"));

	for (WavData const &f : files) {
		ALuint buf;
		alGenBuffers((ALuint)1, &buf);
		alBufferData(buf, AL_FORMAT_STEREO16, f.waveData, f.waveSize,
					 f.fmtData.sampleRate);
		checkALErrors("loading " + f.name + " (in staticSound)");

		buffers[f.name]=buf;
	}
}

void StaticAudio::playSound(std::string name) {}

void StaticAudio::close() {
	alcDestroyContext(context);
	alcCloseDevice(device);
}

// checks for the last error thrown by openal
//
// returns true if no err, returns false if err, and prints to stderr
bool StaticAudio::checkALErrors(std::string location) {
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
