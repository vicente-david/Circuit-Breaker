#include "AIControllerSys.h"
#include "debugUtils/Logger.h"
#include "ecs/Coordinator.h"
#include "ecs/Component.h"
#include "../world/CurveLoader.h"
#include "world/LeaderboardSystem.h"
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>

std::shared_ptr<AIControllerSys> AIControllerSys::registerSystem(std::shared_ptr<Coordinator>& coord) {
	auto system = coord->registerSystem<AIControllerSys>();
	Signature sig;
	sig.set(coord->getComponentType<SparkControls>());
	sig.set(coord->getComponentType<AIController>());
	coord->setSystemSignature<AIControllerSys>(sig);
	return system;
}

void AIControllerSys::update(GameState& game) {
	for (auto const& entity : entities) {
		auto &ai = game.coordinator->getComponent<AIController>(entity);
		auto &controls = game.coordinator->getComponent<SparkControls>(entity);
		auto &transform = game.coordinator->getComponent<Transform>(entity);
		auto& spark = game.coordinator->getComponent<SparkData>(entity);
		auto& body = game.coordinator->getComponent<physx::PxRigidBody*>(entity);
		auto& leaderboard = game.coordinator->getComponent<Leaderboard>(entity);
		
		// update current position index
		int i = ai.currentPosIdx;
		while (glm::distance(transform.pos, ai.route.at(i)) < arrivalRadius) {
			i = (i + 1) % ai.route.size();
		}
		ai.currentPosIdx = i;
		// update target based on current position and the lookAheadSteps value
		ai.targetIdx = (ai.currentPosIdx + ai.lookAheadSteps) % ai.route.size();
	
		if (spark.speed <= 0.02) {
			ai.stillTimer++;
			dbug::log("AI", 2, "[%s] still timer: %.1f", spark.mVehicleName.c_str(), ai.stillTimer);
		}
		else if (ai.stillTimer > 0.0f) ai.stillTimer -= 0.5f;
		if (ai.stillTimer > 4.f) {
			dbug::log("AI", 2, "[%s]: I'm stuck!!", spark.mVehicleName.c_str());
		}
		// Adjust lookahead target based on speed
		if (spark.speed < 16.0f)
			ai.lookAheadSteps = 2;
		else if (spark.speed < 28.0f)
			ai.lookAheadSteps = 4;
		else if (spark.speed < 42.f)
			ai.lookAheadSteps = 6;
		
		
		
		AIDriveContext ctx{ ai, controls, transform, spark, body, 0.0f};

		if (ai.state == IDLE) {
			AI_IDLE(ai, controls, game);
		}
		else if (spark.health < 50.0f) {
			ctx.healthBoostMin = 101.f; // do not use health for boost
			defenseState->run(ctx);
		}
		else if (leaderboard.standings[0] == spark.mVehicleName) {
			ctx.healthBoostMin = 95.0f; // allowed to use health to boost in this state
			maintainState->run(ctx);
		}
		else { // Not in first and not low health
			ctx.healthBoostMin = 85.f;
			overtakeState->run(ctx);
		}

		ai.lastPosIdx = i;

	}
}

// TODO: move this with the rest of the states (?)
void AIControllerSys::AI_IDLE(AIController& ai, SparkControls& controls, GameState& game) {
	// zero out all controls, car will stay idle
	controls.throttle = 0.0f;
	controls.steering = 0.0f;
	controls.brake = 0.0f;
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;
	controls.reset = false;
	dbug::log("AI", 0, "..... idling ....");	// AI only idle when race hasn't started yet
	if (game.raceStart == true) {
		ai.state = DRIVING;
	}
	
	return;
}

// Initialize the curve/path for each state
void AIControllerSys::setStatePaths(std::vector<TrackCurve>& paths) {
	defenseState->path = std::make_unique<TrackCurve>(paths[1]); // heal zone path
	maintainState->path = std::make_unique<TrackCurve>(paths[0]); // regular path
	overtakeState->path = std::make_unique<TrackCurve>(paths[0]);
	return;
}