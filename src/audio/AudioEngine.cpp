#include "AudioEngine.h"
#include "Sound.h"
#include "WavData.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <glm/fwd.hpp>
#include <stdbool.h>
#include <string>
#include <vector>
#include "debugUtils/Logger.h"

// reference:
// https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/
// https://ffainelli.github.io/openal-example/
//

AudioEngine::AudioEngine() {


	dbug::log("AUDIO", 0, "intializing audio" );
	// use the deafult audio device
	device = alcOpenDevice(NULL);

	if (!device) {
		fprintf(stderr, "Default audio device couldn't be opened :(\n");
	}

	// create a al context
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	checkALErrors("creating AL Context");

	ALfloat listenerZeroes[] = {0.0, 0.0, 0.0};
	alListenerfv(AL_POSITION, listenerZeroes);
	checkALErrors("setting position to zero");
	alListenerfv(AL_VELOCITY, listenerZeroes);
	checkALErrors("setting velocity to zero");

	loadSounds();
}

void AudioEngine::loadSounds() {
	sounds.emplace("hiya", WavData("assets/sounds/hiyaMono.wav"));
	// sounds["hiya"].loop = true;
	sounds.emplace("full", WavData("assets/sounds/aaa.wav"));
	sounds.emplace("muteCity", WavData("assets/sounds/muteCityMono.wav"));
}

void AudioEngine::update(double dt) {

	// clean up channels/sources that have finished
	auto remIdx =
		std::remove_if(channels.begin(), channels.end(), [](Sound sound) {
			ALint state;
			alGetSourcei(sound.source, AL_SOURCE_STATE, &state);
			if (state == AL_STOPPED) {
				alDeleteSources(1, &sound.source);
				sound.freed = false;
				return true;
			}

			return false;
		});
	channels.erase(remIdx, channels.end());
}

// updates the location/rotation of the listener location.
// this shold just be the view matrix from a camera
void AudioEngine::updateListenerFrame(glm::mat4 viewMat) {
	listener.viewMatrix = viewMat;
}

// velocity for dopler effect
// this should be given in world coordinates
// this will eventually need to be updated if we do split screen
void AudioEngine::updateListenerVel(float x, float y, float z) {
	listener.velx = x;
	listener.vely = y;
	listener.velz = z;

	// printf("list vel. [%f, %f, %f]\n", x, y, z);
}

// move a sound instance to a specified location in world coordinates
// Note: 3d sound only works with mono audio files. if you use sterio there aren't errors, but it just won't move in 3D
void AudioEngine::updateSoundLoc(Sound s, float x, float y, float z) {
	// point for sound loc
	auto loc = glm::vec4(x, y, z, 1);

	// convert to screen coords
	loc = listener.viewMatrix * loc;

	float alData[3] = {loc.x / 20, loc.y / 20, loc.z / 20};
	alSourcefv(s.source, AL_POSITION, alData);

	// printf("trans loc: [%f, %f, %f]\n", alData[0], alData[1], alData[2]);
}

// update sound velocity for dopler shifting
// this should be in world coordinates
void AudioEngine::updateSoundVel(Sound s, float x, float y, float z) {
	// get velocity relative to the camera (in world coordinates)
	glm::vec4 vel(x - listener.velx, y - listener.vely,
									 z - listener.velz, 0);
	// transform to camera frame
	vel = listener.viewMatrix * vel;
	float alData[3] = {vel.x, vel.y, vel.x};

	// printf("rel vel. [%f, %f, %f]\n", alData[0], alData[1], alData[2]);
	alSourcefv(s.source, AL_VELOCITY, alData);
}

// create a sounds channel to play a sound. you need to call play before it will
// play
Sound AudioEngine::createSound(std::string name) {
	ALuint channel = sounds[name].createSource();
	Sound s(channel, name);

	return s;
}

void AudioEngine::close() {
	for (auto ch : channels) {
		ch.freed = true;
		alDeleteSources(1, &ch.source);
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

	dbug::log("AUDIO", 2, "Open Al Error at location %s\n", location.c_str() );
	switch (err) {
	case AL_INVALID_NAME:

		dbug::log("AUDIO",2,
				"AL_INVALID_NAME: invalid ID was passed to AL function.\n");
		break;

	case AL_INVALID_ENUM:
		dbug::log("AUDIO",2,
				"AL_INVALID_ENUM: invalid enum was passed to AL function.\n");
		break;
	case AL_INVALID_VALUE:
		dbug::log("AUDIO",2,
				"AL_INVALID_VALUE: invalid value was passed to AL function.\n");
		break;
	case AL_INVALID_OPERATION:
		dbug::log("AUDIO",2,
				"AL_INVALID_OPERATION: requested operation is invalid.\n");
		break;
	case AL_OUT_OF_MEMORY:
		dbug::log("AUDIO",2, "AL_OUT_OF_MEMORY: openAL ran out of memory!.\n");
		break;
	default:
		dbug::log("AUDIO",2, "UNKNOWN AL ERROR: good luck :(\n");
		break;
	}
	return false;
}
