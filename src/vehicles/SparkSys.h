#pragma once

#include "GameState.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include "ecs/System.h"
#include "SparkComponents.h"

class SparkSys : public System {

  public:

	void updateSparks(double dt, GameState &gameState);
	static std::shared_ptr<SparkSys>
	registerSystem(std::shared_ptr<Coordinator> &coord);

	EcsEntity createSpark(GameState& game);

	bool init();
	void cleanup();
	void changeEngineDriveParams(const char* vehicleDataPath);

};
