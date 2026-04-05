#pragma once
#include <memory>
#include "ParticleSystem.h"
#include "ParticleEmitter.h"

class ParticleHelper {

public:
	ParticleHelper() {};

	void connectSys(const std::shared_ptr<ParticleSystem>& p) {
		particleSys = p;
		
	}

	void notify(Particle& particle, unsigned int spawnNum) {
		std::cout << "Notified!" << std::endl;
		particleSys->addParticleBurst(particle, spawnNum);
	}
	

private:
	std::shared_ptr<ParticleSystem> particleSys;
};