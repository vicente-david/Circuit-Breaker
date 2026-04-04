#include "ParticleSystem.h"




std::shared_ptr<ParticleSystem>ParticleSystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	// register system
	auto system = coord->registerSystem<ParticleSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	coord->setSystemSignature<RenderingSystem>(sig);

	return system;
}