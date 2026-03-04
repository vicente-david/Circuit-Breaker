#include "AIControllerSys.h"
#include "debugUtils/Logger.h"
#include "ecs/Coordinator.h"
#include "ecs/Component.h"
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

		if (ai.state == IDLE)
			AI_IDLE(ai, controls, transform);
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

	}
}

void AIControllerSys::AI_IDLE(AIController &ai, SparkControls &controls, Transform &transform) {
	// zero out all controls, car will stay idle
	controls.throttle = 0.0f;
	controls.steering = 0.0f;
	controls.brake = 0.0f;
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;
	controls.reset = false;

	// logic to determine if we should start driving
	glm::vec3 targetDistance = ai.targetPos - transform.pos; // vector from the spark to target location
	float distance = glm::length(glm::vec2(targetDistance.x, targetDistance.z)); // get the length of this vector to get the distance
	// if we are not yet within the arrival radius, change to DRIVING state
	if (distance > ai.arrivalRadius) { 
		ai.state = DRIVING;
		dbug::log("AI", 0, "Entity: IDLE -> DRIVING (dist: %.1f)", distance);
	}
	return;
}

void AIControllerSys::AI_DRIVING(AIController& ai, SparkControls& controls, Transform& transform) {
	glm::vec3 targetDistance = ai.targetPos - transform.pos; // vector from the spark to target location
	float distance = glm::length(glm::vec2(targetDistance.x, targetDistance.z)); // get the length of this vector to get the distance

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
		return;
	}

	// ---- STEERING ----
	// compute the signed angle between car forward and direction to target, both projected onto XZ plane
	// Z = forward, X = lateral, Y = vertical 
	glm::vec2 directionToTarget = glm::normalize(glm::vec2(targetDistance.x, targetDistance.z));
	glm::vec2 sparkForward = glm::normalize(glm::vec2(transform.forwardD.x, transform.forwardD.z));

	// 2D cross product gives sin of the angle (sign gives turn direction, + = right, - = left)
	// 2D dot product gives cos of the angle
	float cross = sparkForward.x * directionToTarget.y - sparkForward.y * directionToTarget.x;
	float dot = sparkForward.x * directionToTarget.y + sparkForward.y * directionToTarget.y;
	float angle = std::atan2(cross, dot);

	// map the angle to [-1, 1] steering. pi/2 rads (90 degrees) = full
	// lock before sharpness multiplier
	float steerRaw = (angle / (glm::pi<float>() * 0.5f)) * ai.steeringSharpness;
	controls.steering = glm::clamp(steerRaw, -1.0f, 1.0f);

	// ---- THROTTLE (and brake for now) ----
	if (distance < ai.brakeDistance) {
		// decrease throttle as we approach
		float t = distance / ai.brakeDistance;
		controls.throttle = glm::clamp(t, 0.1f, 1.0f);
		controls.brake = glm::clamp(1.0f - t, 0.0f, 0.5f);
	}
	else {
		controls.throttle = 1.0f;
		controls.brake = 0.0f;
	}

	// zero out the unused controls
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;
	controls.reset = false;

	dbug::log("AI", 0, "Entity: dist = %.1f steer=%.2f throttle = %.2f brake = %.2f curr location = (%.1f, %.1f, %.1f)", distance, controls.steering, controls.throttle, controls.brake, transform.pos.x, transform.pos.y, transform.pos.z);
	return;
}

void AIControllerSys::AI_BRAKING(AIController& ai, SparkControls& controls, Transform& transform) {
	// TODO: probably move braking logic from DRIVING to here, other minor things
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