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
	Entity createSpark(GameState &game, PxVec3 startP, std::string name = "unnamed_spark");

	bool init();
	void cleanup();
	void reloadSparkParams( GameState& gameState);

  private:
	// === Helpers ===
	void checkDeath(SparkData& sData, double dt);
	void checkAirborne(SparkData& sData, double dt);

	void angularResistance(SparkData& sData, PxReal val = 0.05f, double duration = 0);
	void checkAngResistace(SparkData& sData, double dt);

	// Collision
	void sparkCollision(GameState& game);
	void wallCollision(GameState& game);
	void healZoneCheck(GameState& game, double dt);

	// Commands
	void sparkInputs(SparkData& sData, SparkControls& sControls, double dt);
	void reverse(SparkData& sData, SparkControls& sControls);

	// Features
	void updateMaxBoost(SparkData &sData);
	void applyBoost(SparkData &sData, bool useHealth, double dt);
	void boost(SparkData &sData, SparkControls &sControls, double dt);
	void regenBoost(SparkData& sData, double dt);
	
	void applyShimmy(SparkData& sData, bool dir);
	void shimmy(SparkData& sData, SparkControls& sControls, double dt);

	// Handling
	void changeWheelParams(SparkData& sData, PxReal friction, PxReal latFriction, PxReal maxSteerAngle);
	void driftStabilizer(SparkData& sData, SparkControls& sControls);
	void yawStabilizer(SparkData& sData);
	void sparkHandling(SparkData& sData, SparkControls& sControls);

	// Respawn
	void sparkValuesReset(SparkData& sData);
	void respawn(SparkData& sData, SparkControls& sControls, double dt);
};
