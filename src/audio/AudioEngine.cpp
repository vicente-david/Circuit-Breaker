#include "AudioEngine.h"
#include "Sound.h"
#include "WavData.h"
#include "foundation/PxVec3.h"
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

	ALfloat listnerZeroes[] = {0.0, 0.0, 0.0};
	alListenerfv(AL_POSITION, listnerZeroes);
	checkALErrors("setting position to zero");
	alListenerfv(AL_VELOCITY, listnerZeroes);
	checkALErrors("setting velocity to zero");

	loadSounds();
}

void AudioEngine::loadSounds() {
	sounds.emplace("hiya", WavData("assets/sounds/hiyaMono.wav"));
	// sounds["hiya"].loop = true;
	sounds.emplace("full", WavData("assets/sounds/aaa.wav"));
}

void AudioEngine::update(double dt) {
	// test += dt;
	//
	// if (test > 1) {
	// 	Sound i = createSound("hiya");
	// 	i.start();
	// 	test = 0;
	// }

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

void AudioEngine::updateListnerLoc(float x, float y, float z) {
	listner.x = x;
	listner.y = y;
	listner.z = z;
}
void AudioEngine::updateListnerVel(float x, float y, float z) {
	listner.velx = x;
	listner.vely = y;
	listner.velz = z;
}

// move a sound to a specified location, and give a velocity for dopler stuff
void AudioEngine::updateSoundLoc(Sound s, float x, float y, float z) {
	float loc[3] = {
		x - listner.x,
		y - listner.y,
		z - listner.z,
	};
	alSourcefv(s.source, AL_POSITION, loc);
}

void AudioEngine::updateSoundVel(Sound s, float x, float y, float z) {
	float vel[3] = {
		x - listner.x,
		y - listner.y,
		z - listner.z,
	};
	alSourcefv(s.source, AL_VELOCITY, vel);
}
// create a sounds channel for the sound, but don't actually play it
Sound AudioEngine::createSound(std::string name) {
	ALuint channel = sounds[name].createSource();
	Sound s(channel, name);

	return s;
}

// ALuint AudioEngine::playSound(std::string name, float x, float y, float z) {
// 	auto source = createSound(name);
// 	if (source == -1) {
// 		return -1;
// 	}
//
// 	if (sounds[name].is3D) {
// 		float pos[3] = {x, y, z};
// 		alSourcefv(source, AL_POSITION, pos);
// 	} else {
// 		fprintf(stderr,
// 				"can't play stereo audio '%s' in 3D (played normally
// instead)\n", 				name.c_str());
// 	}
// 	alSourcePlay(source);
// 	checkALErrors("playing " + name);
// 	return source;
// }
//
// ALuint AudioEngine::playSound(std::string name) {
// 	auto ch = createSound(name);
// 	alSourcePlay(ch);
// 	checkALErrors("playing " + name);
//
// 	return ch;
// }

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
