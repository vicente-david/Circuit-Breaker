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
#include "AIStateComponent.h"

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
		auto& lapProg = game.coordinator->getComponent<LapCounter>(entity);
		auto& stateP = game.coordinator->getComponent<AIDriveStateP>(entity);
		
		// update current position index
		int i = ai.currentPosIdx;
		while (glm::distance(transform.pos, ai.route.at(i)) < arrivalRadius) {
			i = (i + 1) % ai.route.size();
		}
		ai.currentPosIdx = i;
		// update target based on current position and the lookAheadSteps value
		ai.targetIdx = (ai.currentPosIdx + ai.lookAheadSteps) % ai.route.size();

		// check progress of the spark
		ai.checkProgTimer.update(*game.frameTime);

		if (ai.checkProgTimer.completedTimer() && !spark.isDead && spark.ghostTimer <= 1e-4) {

			if (ai.currentPosIdx == ai.logIdx) {
				// no significant lap progress made since last check
				dbug::log("AI_RECOVER", 0, "[%s]: I'm stuck!!", spark.mVehicleName.c_str());

				// check if an attempt to recover was made 
				if (ai.recoverClock.activeTimer()) {
					
					// update time spent trying to recover
					ai.recoverClock.update(*game.frameTime);
					ai.recoverAttempt = true; // set state to do recover actions
					dbug::log("AI_RECOVER", 0, "[%s]: attempt recover", spark.mVehicleName.c_str());
	
					if (ai.recoverClock.completedTimer()) {
						// if the timer just completed, exit recovery state
						ai.recoverAttempt = false;
						spark.inReverse = false;
						ai.checkProgTimer.start(3.0); // reset the progress timer

					}
				}
				
				else {
					// recovery attempt failed, respawn
					controls.reset = true;
					ai.recoverAttempt = false;
					ai.recoverDir = NONE;
					spark.inReverse = false;
					controls.throttle = 1.f;
					ai.checkProgTimer.start(5.0); //restart timer (give a longer time)
					ai.recoverClock.start(1.0);
				}

			}
			else {
				//ai.recoverAttempt = false;
				ai.recoverDir = NONE;
				spark.inReverse = false;
				controls.brake = 0.0;
				controls.throttle = 1.0f;
				ai.checkProgTimer.start(3.0); //restart regular timer
				ai.recoverClock.start(1.0);
			}
			ai.logIdx = ai.currentPosIdx;
			
		}
		

		// If active, update attack cooldown timer
		if (ai.attackCooldown.activeTimer()) {
			ai.attackCooldown.update(*game.frameTime);
		}


		// Adjust lookahead target based on speed
		if (ai.respawnRecoverTimer > 0.0f) {
			// respawn recovery
			ai.lookAheadSteps = 1;
			dbug::log("AIPATH", 0, "[%s] respawn recovery: %.1f, steering: %.3f", spark.mVehicleName.c_str(), ai.respawnRecoverTimer, controls.steering);
			ai.respawnRecoverTimer -= 0.1f;
		}
		else if (spark.speed < 16.0f)
			ai.lookAheadSteps = 2;
		else if (spark.speed < 28.0f)
			ai.lookAheadSteps = 4;
		else if (spark.speed < 42.f)
			ai.lookAheadSteps = 6;
		
		AIDriveContext ctx{ ai, controls, transform, spark, body, 0.0f };
		

		if (ai.state == IDLE) {
			AI_IDLE(ai, controls, game);
		}
		else if (ai.recoverAttempt) {
			recoverState->run(ctx, stateP);
		}
		else if (spark.health < 50.0f) {
			ctx.healthBoostMin = 101.f; // do not use health for boost
			defenseState->run(ctx, stateP);
		}
		else if (leaderboard.standings[0] == spark.mVehicleName) {
			ctx.healthBoostMin = 95.0f; // allowed to use health to boost in this state
			maintainState->run(ctx, stateP);
		}
		else { // Not in first and not low health
			ctx.healthBoostMin = 85.f;
			overtakeState->run(ctx, stateP);
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
