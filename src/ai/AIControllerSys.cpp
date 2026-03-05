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

		// update current position index
		int i = ai.currentPosIdx;
		while (glm::distance(transform.pos, ai.route.at(i)) < ai.arrivalRadius) {
			i = (i + 1) % ai.route.size();
		}
		ai.currentPosIdx = i;

		// Update target index only if progress was made to current target
		float distToTarget = glm::distance(ai.route.at(ai.targetIdx), transform.pos);
		
		float halfDist = glm::distance(ai.route.at(ai.targetIdx), ai.route.at(ai.currentPosIdx)) / 2.f;
		if (distToTarget <= halfDist) {
			
		}
		ai.targetIdx = (ai.currentPosIdx + lookAheadSteps) % ai.route.size();
	
		dbug::log("AI", 1, "CURRENT IDX: %d, TARGET IDX: %d", ai.currentPosIdx, ai.targetIdx);
		
		
		if (ai.state == IDLE)
			AI_IDLE(ai, controls, game);
		else if (ai.state == DRIVING)
			AI_DRIVING(ai, controls, transform);
		else if (ai.state == BRAKING)
			AI_BRAKING(ai, controls, transform);
		else if (ai.state == DRIFTING)
			AI_DRIFTING(ai, controls, transform);
		else if (ai.state == BOOSTING)
			AI_BOOSTING(ai, controls, transform);
		else if (ai.state == ATTACKING)
			AI_ATTACKING(ai, controls, transform);

		glm::vec3 indLoc = ai.route.at(ai.currentPosIdx);
		glm::vec3 targLoc = ai.route.at(ai.targetIdx);
		//dbug::log("AI", 1, "LOC: (%.1f, %.1f, %.1f) IDX: (%.1f, %.1f, %.1f) TARGET: (%.1f, %.1f, %.1f)",
		//	transform.pos.x, transform.pos.y, transform.pos.z,
		//	indLoc.x, indLoc.y, indLoc.z, targLoc.x, targLoc.y, targLoc.z);


	}
}


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

	// AI only idle when race hasn't started yet
	if (game.raceStart == true) {
		ai.state = DRIVING;
	}

	//// logic to determine if we should start driving
	//glm::vec3 targetPos = ai.route.at(ai.targetIdx);
	//glm::vec3 vectorToTarget = targetPos - transform.pos; // vector from the spark to target location
	//vectorToTarget.y = 0.0f; // zero out the Y coordinate so that we get a vector on the XZ-plane
	//float distance = glm::length(vectorToTarget); // get the length of this vector to get the distance
	//// if we are not yet within the arrival radius, change to DRIVING state
	//if (distance > ai.arrivalRadius) { 
	//	ai.state = DRIVING;
	//	dbug::log("AI", 0, "Entity: IDLE -> DRIVING (dist: %.1f)", distance);
	//}

	
	return;
}

void AIControllerSys::AI_DRIVING(AIController& ai, SparkControls& controls, Transform& transform) {
	glm::vec3 targetPos = ai.route.at(ai.targetIdx);
	glm::vec3 vectorToTarget = targetPos - transform.pos; // vector from the spark to target location
	//vectorToTarget.y = 0.0f; // zero out the Y coordinate so that we get a vector on the XZ-plane
	float distance = glm::length(vectorToTarget); // get the length of this vector to get the distance

	// if we've arrived, stop and change to IDLE state
	if (distance <= ai.arrivalRadius) {
		controls.throttle = 0.0f;
		controls.steering = 0.0f;
		controls.brake = 1.0f;
		controls.reverse = 0.0f;
		controls.boost = false;
		controls.shimmyL = false;
		controls.shimmyR = false;
		controls.reset = false;

		ai.state = IDLE;
		dbug::log("AI", 0, "Entity: DRIVING -> IDLE (dist: %.1f)", distance);
		return;
	}

	// ---- STEERING ----
	// compute the signed angle between car forward and direction to target, both projected onto XZ plane
	// Z = forward, X = lateral, Y = vertical 
	glm::vec3 unitVectorToTarget = glm::normalize(vectorToTarget);
	glm::vec3 sparkForward = glm::normalize(transform.forwardD);

	// 2D cross product gives sin of the angle (sign gives turn direction, + = right, - = left)
	// 2D dot product gives cos of the angle
	float cross = sparkForward.x * unitVectorToTarget.z - sparkForward.z * unitVectorToTarget.x;
	float dot = sparkForward.x * unitVectorToTarget.x + sparkForward.z * unitVectorToTarget.z;
	float angle = glm::atan(cross, dot);

	// map the angle to [-1, 1] steering
	// lock before sharpness multiplier
	float steerRaw = -(angle / (glm::pi<float>() / 2)); // *ai.steeringSharpness;
	controls.steering = glm::clamp(steerRaw, -1.0f, 1.0f);

	// ---- THROTTLE ----
	if (distance <= ai.brakeDistance) {
		// close enough to start braking, hand off to BRAKING state
//		ai.state = BRAKING;
		dbug::log("AI", 0, "Entity: DRIVING -> BRAKING (dist: %.1f)", distance);
		return;
	}
	else {
		float headingAlignment = glm::clamp(dot, 0.0f, 1.0f);
		float t = distance / ai.brakeDistance;
		controls.throttle = glm::clamp(t, 0.3f, 0.5f);
		controls.brake = 0.0f;
	}

	// zero out the unused controls
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;

	dbug::log("AI", 0, "Entity: dist = %.1f steer=%.2f throttle = %.2f brake = %.2f curr location = (%.1f, %.1f, %.1f)", distance, controls.steering, controls.throttle, controls.brake, transform.pos.x, transform.pos.y, transform.pos.z);
	return;
}

void AIControllerSys::AI_BRAKING(AIController& ai, SparkControls& controls, Transform& transform) {
	glm::vec3 targetPos = ai.route.at(ai.targetIdx);
	glm::vec3 vectorToTarget = targetPos - transform.pos;
	vectorToTarget.y = 0.0f; // zero out the Y coordinate so that we get a vector on the XZ-plane
	float distance = glm::length(vectorToTarget);

	// if we've arrived, stop and go to IDLE
	/*if (distance <= ai.arrivalRadius) {
		controls.throttle = 0.0f;
		controls.steering = 0.0f;
		controls.brake = 1.0f;
		controls.reverse = 0.0f;
		controls.boost = false;

		ai.state = IDLE;
		dbug::log("AI", 0, "Entity: BRAKING -> IDLE (dist: %.1f)", distance);
		return;
	}*/

	// ---- STEERING ----
	glm::vec3 unitVectorToTarget = glm::normalize(vectorToTarget);
	glm::vec3 sparkForward = glm::normalize(transform.forwardD);

	float cross = sparkForward.x * unitVectorToTarget.z - sparkForward.z * unitVectorToTarget.x;
	float dot = sparkForward.x * unitVectorToTarget.x + sparkForward.z * unitVectorToTarget.z;
	float angle = glm::atan(cross, dot);

	float steerRaw = (angle / (glm::pi<float>() / 2)) * ai.steeringSharpness;
	controls.steering = glm::clamp(steerRaw, -1.0f, 1.0f);

	// ---- ALIGNING VEHICLE ----
	// 1.0 = pointing right at it, 0.0 = perpendicular, -1.0 = pointing away
	float headingAlignment = glm::clamp(dot, 0.0f, 1.0f);
	float t = distance / ai.brakeDistance;

	// ---- DECREASING THROTTLE ----
	// coast in with decreasing throttle, cut throttle and apply brakes close to the target
	if (distance <= ai.arrivalRadius * 2) {
		// very close to target: cut throttle entirely and brake to stop
		controls.throttle = 0.0f;
		controls.brake = 0.05f;
	}
	else {
		// further out: coast in with decreasing throttle, scaled by heading alignment
		controls.throttle = glm::clamp(t * headingAlignment, 0.1f, 1.0f);
		controls.brake = 0.0f;
	}

	// zero out unused controls
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;

	dbug::log("AI", 1, "Entity: BRAKING dist = %.1f steer=%.2f throttle = %.2f brake = %.2f", distance, controls.steering, controls.throttle, controls.brake);
	return;
}

void AIControllerSys::AI_DRIFTING(AIController& ai, SparkControls& controls, Transform& transform) {
	// TODO: drift logic when drifting is implemented
	return;
}

void AIControllerSys::AI_BOOSTING(AIController& ai, SparkControls& controls, Transform& transform) {
	// TODO: add boost logic for AI
	return;
}

void AIControllerSys::AI_ATTACKING(AIController& ai, SparkControls& controls, Transform& transform) {
	// TODO: add logic for attacking other players
	return;
}