#pragma once

#include "GameState.h"
#include "ai/AISparkComponents.h"
#include "vehicles/SparkComponents.h"
#include "ecs/System.h"
#include "AIStates.h"


// this system is responsible for controlling the AI sparks
// practically identical to the ControllerSys but with AIControls instead of player input
class AIControllerSys : public System {
	public:
		void update(GameState& game);
		static std::shared_ptr<AIControllerSys> registerSystem(std::shared_ptr<Coordinator>& coord);
		void setStatePaths(std::vector<TrackCurve>& paths);

		void AI_IDLE(AIController &ai, SparkControls &controls, GameState& game);
		

private:
	
	float arrivalRadius = 20.0f; // how close a spark needs to be from the targetPos to consider it as "arrived"
	std::unique_ptr<AIState> defenseState = std::make_unique<DefenseState>();
	std::unique_ptr<AIState> overtakeState = std::make_unique<OvertakeState>();
	std::unique_ptr<AIState> maintainState = std::make_unique<MaintainState>();
	std::unique_ptr<AIState> recoverState = std::make_unique<RecoverState>();
};