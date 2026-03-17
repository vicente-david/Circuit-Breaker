#pragma once

#include "GameState.h"
#include "SparkComponents.h"

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
	void reverse(SparkData& sData, SparkControls& sControls);

	// helper functions for doing the movements
	void applyShimmy(SparkData &sData, bool dir);
	void shimmy(SparkData& sData, SparkControls& sControls, double dt);

	void updateMaxBoost(SparkData &sData);
	void applyBoost(SparkData &sData);
	void boost(SparkData &sData, SparkControls &sControls, double dt);
	
	// couldn't figure out a way to use the logic from RespawnSystem, so I just copied it here
	void respawnSpark(PxRigidBody *rBody, PxTransform respawnPose);
	PxTransform getRespawnPose(Entity entity, GameState &game); 
};
