
// helper to register the system
#include "vehicles/SparkSoundSys.h"
#include "GameState.h"
#include "debugUtils/Logger.h"
#include "ecs/Component.h"
#include "ecs/Coordinator.h"
#include "vehicles/ControllerSys.h"
#include "vehicles/SparkComponents.h"
#include <cstdio>
#include <memory>

void SparkSoundSys::updateSounds(double dt, GameState &game) {
	for (auto const &entity : entities) {
		auto &controls = game.coordinator->getComponent<SparkControls>(entity);
		auto &sData = game.coordinator->getComponent<SparkData>(entity);
		auto &sounds = game.coordinator->getComponent<SparkSounds>(entity);
		auto &rBody = game.coordinator->getComponent<PxRigidBody *>(entity);

		float pitch = 1;
		pitch += rBody->getLinearVelocity().magnitude() / 40.f;
		if (sData.isBoosting) {
			pitch += sounds.currBoostPitch;
			sounds.currBoostPitch += dt * sounds.boostPitchSpeed;
			if (sounds.currBoostPitch > sounds.maxBoostPitch) {
				sounds.currBoostPitch = sounds.maxBoostPitch;
			}
		} else {
			sounds.currBoostPitch = 0;
		}

		if (controls.throttle > 0 || sData.isBoosting) {
			sounds.engine->start();
			sounds.engine->pitchMulti(pitch);
			if (sData.isHuman) {
				dbug::log("AUDIO", -1, "vel:%f  engine pitch: %f",
						  rBody->getLinearVelocity().magnitude(), pitch);
			}
		} else {
			sounds.engine->pause();
		}

		sounds.engine->updateFromRbody(rBody);
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
	sig.set(coord->getComponentType<physx::PxRigidBody *>());
	coord->setSystemSignature<SparkSoundSys>(sig);

	return system;
}
