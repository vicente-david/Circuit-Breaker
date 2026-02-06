#include "AudioEngine.h"
#include "WavData.h"
#include <al.h>
#include <alc.h>
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
float x = -10;
AudioEngine::AudioEngine() {

	// use the deafult audio device
	device = alcOpenDevice(NULL);

	if (!device) {
		fprintf(stderr, "Default audio device couldn't be opened :(\n");
	}

	// create a al context (not really sure what that actually is though_
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	checkALErrors("creating AL Context");

	loadSounds();
}

void AudioEngine::loadSounds() {

	// std::vector<WavData> files;
	// files.push_back(WavData("hiya", "assets/sounds/hiya.wav"));
	// files.push_back(WavData("full", "assets/sounds/aaa.wav"));
	//
	// for (WavData const &f : files) {
	// 	ALuint buf;
	// 	alGenBuffers((ALuint)1, &buf);
	// 	alBufferData(buf, AL_FORMAT_STEREO16, f.waveData, f.dataSize,
	// 				 f.fmtData.sampleRate);
	// 	checkALErrors("loading " + f.name + " (in staticSound)");
	//
	// 	soundBuffs[f.name] = buf;
	// }
	sounds.emplace("hiya", WavData("assets/sounds/hiyaMono.wav"));
	// sounds["hiya"].loop = true;
	sounds.emplace("full", WavData("assets/sounds/aaa.wav"));
}

void AudioEngine::update(double dt) {
	test += dt;

	if (test > 1) {
		ALuint i = playSound("hiya", x,0,0);
		test = 0;
		x += 1;
	}

	// clean up channels/sources that have finished
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

// create a sounds channel for the sound, but don't actually play it
ALuint AudioEngine::createSound(std::string name) {
	ALuint channel = sounds[name].createSource();
	if (channel == -1) {
		return -1;
	}
	channels.push_back(channel);
	return channel;
}
ALuint AudioEngine::playSound(std::string name, float x, float y, float z) {
	auto source = createSound(name);
	if (source == -1) {
		return -1;
	}

	if (sounds[name].is3D) {
		float pos[3] = {x, y, z};
		alSourcefv(source, AL_POSITION, pos);
	} else {
		fprintf(stderr,
				"can't play sterio audio '%s' in 3D (played normally instead)\n",
				name.c_str());
	}
	alSourcePlay(source);
	checkALErrors("playing " + name);
	return source;
}

ALuint AudioEngine::playSound(std::string name) {
	auto ch = createSound(name);
	alSourcePlay(ch);
	checkALErrors("playing " + name);

	return ch;
}

void AudioEngine::close() {
	for (auto ch : channels) {
		alDeleteSources(1, &ch);
	}
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
