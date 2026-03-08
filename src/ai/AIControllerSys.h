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
	float curveBrakeThresh = 0.40f; // minimum angle of turn for spark to decrease speed. Allows spark to increase speed indefinitely on any path shallower than this.
	float maxTargetSpeed = 14.0f; // Max speed for target speed calculated based on angle of turn (when angle of turn is above threshold)
	float curveBoostThresh = 0.10f; // max angle for ai to consider boosting

	void calcSteering(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
};