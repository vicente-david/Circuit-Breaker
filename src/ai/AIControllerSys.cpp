#include "AIControllerSys.h"
#include "debugUtils/Logger.h"
#include "ecs/Coordinator.h"
#include "ecs/Component.h"
#include "../world/CurveLoader.h"
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
		
		// update current position index
		int i = ai.currentPosIdx;
		while (glm::distance(transform.pos, ai.route.at(i)) < arrivalRadius) {
			i = (i + 1) % ai.route.size();
		}
		ai.currentPosIdx = i;
		// update target based on current position and the lookAheadSteps value
		ai.targetIdx = (ai.currentPosIdx + ai.lookAheadSteps) % ai.route.size();
		
		
		// Adjust lookahead target based on speed
		if (spark.speed < 10.0f)
			ai.lookAheadSteps = 4;
		else if (spark.speed < 16.0f)
			ai.lookAheadSteps = 6;
		else if (spark.speed < 32.f)
			ai.lookAheadSteps = 8;
		else if (spark.speed < 40.f)
			ai.lookAheadSteps = 12;
		else if (spark.speed >= 48.f)
			ai.lookAheadSteps = 20;
		
		
		AIDriveContext ctx{ ai, controls, transform, spark, body };

		if (ai.state == IDLE) {
			AI_IDLE(ai, controls, game);
		}
		else if (spark.health < 50.0f) {
			defenseState->run(ctx);
		}
		else if (spark.health <= 100.f) { // replace this condition with 'if not in first'
			overtakeState->run(ctx);
		}
		else { // if in first
			//maintainState->run(ctx);
		}

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
