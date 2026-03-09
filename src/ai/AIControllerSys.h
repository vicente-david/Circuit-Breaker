#pragma once

#include "GameState.h"
#include "ai/AISparkComponents.h"
#include "vehicles/SparkComponents.h"
#include "ecs/System.h"

// this system is responsible for controlling the AI sparks
// practically identical to the ControllerSys but with AIControls instead of player input
class AIControllerSys : public System {
	public:
		void update(GameState& game);
		static std::shared_ptr<AIControllerSys> registerSystem(std::shared_ptr<Coordinator>& coord);

		void AI_IDLE(AIController &ai, SparkControls &controls, GameState& game);
		void AI_DRIVING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
		void AI_BRAKING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
		void AI_DRIFTING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
		void AI_BOOSTING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
		void AI_ATTACKING(AIController& ai, SparkControls& controls, Transform& transform);

private:
	
	float arrivalRadius = 14.0f; // how close a spark needs to be from the targetPos to consider it as "arrived"

	void calcSteering(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
};