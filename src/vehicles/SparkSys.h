#pragma once

#include "GameState.h"
#include "PxRigidBody.h"
#include "SparkComponents.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include "ecs/System.h"

// this updates the sparks and turns the controls to actual movements and
// gameplay.
class SparkSys : public System {

  public:
	static std::shared_ptr<SparkSys>
	registerSystem(std::shared_ptr<Coordinator> &coord);

	// updates all the sparks in the game
	void updateSparks(double dt, GameState &gameState);


	// very convinient function to just make a spark instead of needing to go
	// though whatever physX makes you to do
	Entity createSpark(GameState &game, PxVec3 startP);

	bool init();
	void cleanup();
	void reloadSparkParams( GameState& gameState);

  private:
	// Helpers
	void sparkCollision(GameState& game);
	void wallCollision(GameState& game);

	void sparkInputs(SparkData& sData, SparkControls& sControls, double dt);

	// helper functions for doing the movements
	void shimmy(PxRigidBody *rBody, SparkData &sData, bool rightDir);
	void updateMaxBoost(SparkData &sData);
	void applyBoost(SparkData &sData);
	void boost(SparkData &sData, SparkControls &sControls, double dt);
	void respawn(PxRigidBody *rBody);
};
