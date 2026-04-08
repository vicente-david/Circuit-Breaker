
#include "Sound.h"
#include "AudioEngine.h"
#include <AL/al.h>
#include <cstdio>
#include <string>

Sound::Sound(ALuint source, std::string name) {
	this->source = source;
	soundName = name;
	if (source == -1) {
		freed = true;
	}
}

// start, but it applies position and velocity for you
void Sound::start(AudioEngine &audio) {
	audio.updateSoundLoc(*this);
	audio.updateSoundVel(*this);
	start();
}
// starts the sound so it will start playing
void Sound::start() {
	if (freed || playing) {
		return;
	}
	playing = true;
	alSourcePlay(source);
	AudioEngine::checkALErrors("playing " + soundName);
}

void Sound::pause() {
	if (freed || !playing) {
		return;
	}
	playing = false;
	alSourcePause(source);
	AudioEngine::checkALErrors("pausing " + soundName);
}

void Sound::stop() {
	stopped = true;
	if (freed) {
		return;
	}
	playing = false;
	alSourceStop(source);
	AudioEngine::checkALErrors("stopping " + soundName);
}

void Sound::pitchMulti(float pitch) {
	if (freed) {
		return;
	}
	alSourcef(source, AL_PITCH, pitch);
	AudioEngine::checkALErrors("setting pitch " + soundName);
}

void Sound::volume(float gain) {
	if (freed) {
		return;
	}
	if (gain > 1) {
		alSourcef(source, AL_MAX_GAIN, gain);
	}
	alSourcef(source, AL_GAIN, gain);
	AudioEngine::checkALErrors("setting volume " + soundName);
}
void Sound::updateFromRbody(physx::PxRigidBody *rBody) {
	auto pos = rBody->getGlobalPose().p;
	position = glm::vec3(pos.x, pos.y, pos.z);

	auto vel = rBody->getLinearVelocity();
	velocity = glm::vec3(vel.x, vel.y, vel.z);
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
