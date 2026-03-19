
// helper to register the system
#include "GameState.h"
#include "ecs/Coordinator.h"
#include "vehicles/SparkComponents.h"
#include <cstdio>
#include <memory>
#include "vehicles/SparkSoundsSys.h"


void SparkSoundSys::updateSounds(double dt, GameState& game){
	for (auto const &entity : entities) {
		// // update sound
		// auto &sound = game.coordinator->getComponent<Sound>(entity);
		// auto pos = rBody->getGlobalPose().p;
		// rBody->getLinearVelocity();
		// sound.position = glm::vec3(pos.x, pos.y, pos.z);
		// auto vel = rBody->getGlobalPose().p;
		// sound.position = glm::vec3(vel.x, vel.y, vel.z);
	}

}

// helper to register system
std::shared_ptr<SparkSoundSys>
SparkSoundSys::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<SparkSoundSys>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<SparkData>());
	sig.set(coord->getComponentType<SparkSounds>());
	sig.set(coord->getComponentType<SparkControls>());
	coord->setSystemSignature<SparkSoundSys>(sig);

	return system;
}
