#pragma once
#include <memory>
#include "ParticleSystem.h"
#include "ParticleEmitter.h"

// The whole purpose of this class is to tell the particle system when to make particles without causing circular dependencies
class ParticleHelper {

public:
	ParticleHelper() {};

	void connectSys(const std::shared_ptr<ParticleSystem>& p) {
		particleSys = p;
	}

	void notifyDMG(Particle& particle, unsigned int spawnNum) {
		particleSys->addDmgParticles(particle, spawnNum);
	}

	void notifyBST(Particle& particle, unsigned int spawnNum) {
		particleSys->addBoostParticles(particle, spawnNum);
	}

	void notifyDRF(Particle& particle, unsigned int spawnNum) {
		particleSys->addDriftParticles(particle, spawnNum);
	}
	

private:
	std::shared_ptr<ParticleSystem> particleSys;
};