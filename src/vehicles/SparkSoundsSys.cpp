
// helper to register the system
#include "vehicles/SparkSoundsSys.h"
#include "GameState.h"
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

		if (controls.throttle > 0) {
			sounds.engine->start();
		} else {
			sounds.engine->pause();
		}

		if (sData.isBoosting) {
			if (!sounds.boost->playing) {
				sounds.boost = game.audio->createSound("boost");
				sounds.boost->start();
			}
		} else {
			sounds.boost->stop();
		}

		if (sData.health < 10) {
			sounds.lowHealth->start();
		} else {
			sounds.lowHealth->pause();
		}

		// if(controls.throttle>0){
		// 	alSourcef(sounds.engine.source, AL_GAIN, controls.throttle);
		// }

		// // update sound
		// auto &sound = game.coordinator->getComponent<Sound>(entity);
		// rBody->getLinearVelocity();
		auto pos = rBody->getGlobalPose().p;
		sounds.boost->position = glm::vec3(pos.x, pos.y, pos.z);
		sounds.engine->position = glm::vec3(pos.x, pos.y, pos.z);
		sounds.lowHealth->position = glm::vec3(pos.x, pos.y, pos.z);

		// printf("loc [%f, %f, %f]\n", pos.x, pos.y, pos.z);
		auto vel = rBody->getLinearVelocity();
		sounds.boost->velocity = glm::vec3(vel.x, vel.y, vel.z);
		sounds.engine->velocity = glm::vec3(vel.x, vel.y, vel.z);
		sounds.lowHealth->velocity = glm::vec3(vel.x, vel.y, vel.z);
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
