#include "AudioEngine.h"
#include "WavData.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <stdbool.h>
#include <string>
#include <unistd.h>
#include <vector>

// reference:
// https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/
// https://ffainelli.github.io/openal-example/
//
// This class is a global singleton, play sound from anywhere type thing. (good
// for UI/music and such) a proper audio system for doing dopler/volume based on
// entity locations will probably be a seperate thing
AudioEngine::AudioEngine() {

	// use the deafult audio device
	device = alcOpenDevice(NULL);

	if (!device) {
		fprintf(stderr, "Default audio device couldn't be opened :(\n");
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
}

void AudioEngine::loadSounds() {

	std::vector<WavData> files;
	files.push_back(WavData("hiya", "assets/sounds/hiya.wav"));
	files.push_back(WavData("full", "assets/sounds/aaa.wav"));
	files.push_back(WavData("music", "assets/sounds/house.wav"));

	for (WavData const &f : files) {
		ALuint buf;
		alGenBuffers((ALuint)1, &buf);
		alBufferData(buf, AL_FORMAT_STEREO16, f.waveData, f.waveSize,
					 f.fmtData.sampleRate);
		checkALErrors("loading " + f.name + " (in staticSound)");

		soundBuffs[f.name] = buf;
	}
}

void AudioEngine::update(double dt) {
	test += dt;

	if (test > 1) {
		playSound("music");
		ALuint i = playSound("full");
		printf("ch:%d\n", i);
		test = 0;
	}

	// clean up channels/sources that are finished
	auto remIdx =
		std::remove_if(channels.begin(), channels.end(), [](ALuint source) {
			ALint state;
			alGetSourcei(source, AL_SOURCE_STATE, &state);
			if (state == AL_STOPPED) {
				alDeleteSources(1, &source);
				return true;
			}

			return false;
		});
	channels.erase(remIdx, channels.end());
}
ALuint AudioEngine::playSound(std::string name) {

	ALuint channel;
	alGenSources((ALuint)1, &channel);
	checkALErrors("source gen");
	alSourcei(channel, AL_LOOPING, AL_FALSE);
	checkALErrors("set no looping");

	alSourcei(channel, AL_BUFFER, soundBuffs[name]);
	checkALErrors("Sending audio bufer");
	alSourcePlay(channel);
	checkALErrors("playing " + name);

	channels.push_back(channel);
	printf("ch#:%zu\n", channels.size());

	return channel;
}

void AudioEngine::close() {
	alcDestroyContext(context);
	alcCloseDevice(device);
}

// checks for the last error thrown by openal
//
// returns true if no err, returns false if err, and prints to stderr
bool AudioEngine::checkALErrors(std::string location) {
	ALenum err = alGetError();

	if (err == AL_NO_ERROR) {
		return true;
	}
	fprintf(stderr, "OpenAl Error at location: %s\n", location.c_str());
	switch (err) {
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
