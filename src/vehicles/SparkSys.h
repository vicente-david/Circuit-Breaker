#pragma once

#include "GameState.h"
#include "PxRigidBody.h"
#include "SparkComponents.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include "ecs/System.h"

class SparkSys : public System {

  public:
	void updateSparks(double dt, GameState &gameState);
	static std::shared_ptr<SparkSys>
	registerSystem(std::shared_ptr<Coordinator> &coord);

	void shimmy(PxRigidBody *rBody, SparkData &sData, bool rightDir);
	void boost(PxRigidBody *rBody, SparkData&sData);
	void respawn(PxRigidBody* rBody);

	EcsEntity createSpark(GameState &game);

	bool init();
	void cleanup();
	void changeEngineDriveParams(const char *vehicleDataPath);
};
