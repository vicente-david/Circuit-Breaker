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

	void notify(Particle& particle, unsigned int spawnNum, glm::vec3 fwd) {
		std::cout << "Notified!" << std::endl;
		particleSys->addParticleBurst(particle, spawnNum, fwd);
	}
	

private:
	std::shared_ptr<ParticleSystem> particleSys;
};