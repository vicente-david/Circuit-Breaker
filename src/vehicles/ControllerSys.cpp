
#include "vehicles/ControllerSys.h"
#include "GameState.h"
#include "InputSystem.h"
#include "ecs/Coordinator.h"
#include "vehicles/SparkComponents.h"

std::shared_ptr<ControllerSys>
ControllerSys::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<ControllerSys>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<SparkControls>());
	sig.set(coord->getComponentType<HumanController>());
	// sig.set(coord->getComponentType<physx::PxRigidDynamic *>());
	coord->setSystemSignature<ControllerSys>(sig);

	return system;
}

void ControllerSys::update(GameState &game) {
	for (auto const &entity : entities) {
		Actions &input = game.inputActions;
		SparkControls &sControl =
			game.coordinator->getComponent<SparkControls>(entity);

		SparkData &sData = game.coordinator->getComponent<SparkData>(entity);
		sControl.brake = input.moveBackward;
		sControl.driftMode = input.driftMode;
		sControl.throttle = input.moveForward;
		sControl.steering = input.xRotation;
		sControl.boost = input.boost;
		sControl.shimmyL = input.shimmyLeft;
		sControl.shimmyR = input.shimmyRight;
		sControl.reset = input.respawn;
		sControl.boost = input.boost;

		// I feel like this logic should not be in this file. Should move to SparkSys.cpp
		// activate health override when you start boosting with no boost left
		if (!input.boost) {
			sControl.boostWithHealth = false;
		} else if (input.boostJustPressed && sData.boost < 1) {
			sControl.boostWithHealth = true;
		}
		sControl.reload = input.reload;
	}
}
