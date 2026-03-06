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
		
		// update current position index
		int i = ai.currentPosIdx;
		while (glm::distance(transform.pos, ai.route.at(i)) < ai.arrivalRadius) {
			i = (i + 1) % ai.route.size();
		}
		ai.currentPosIdx = i;
		// update target based on current position and the lookAheadSteps value
		ai.targetIdx = (ai.currentPosIdx + ai.lookAheadSteps) % ai.route.size();
		
		dbug::log("AI", 0, "CURRENT IDX: %d, TARGET IDX: %d", ai.currentPosIdx, ai.targetIdx);
		
		// Adjust lookahead target based on speed
		if (spark.speed < 6.0f)
			ai.lookAheadSteps = 2;
		else if (spark.speed <= 12.0f)
			ai.lookAheadSteps = 4;
		else if (spark.speed > 12.0f)
			ai.lookAheadSteps = 6;

		if (ai.state == IDLE)
			AI_IDLE(ai, controls, game);
		else if (ai.state == DRIVING)
			AI_DRIVING(ai, controls, transform, spark);
		else if (ai.state == BRAKING)
			AI_BRAKING(ai, controls, transform, spark);
		else if (ai.state == DRIFTING)
			AI_DRIFTING(ai, controls, transform);
		else if (ai.state == BOOSTING)
			AI_BOOSTING(ai, controls, transform, spark);
		else if (ai.state == ATTACKING)
			AI_ATTACKING(ai, controls, transform);

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
	
	return;
}

void AIControllerSys::AI_DRIVING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	glm::vec3 targetPos = ai.route.at(ai.targetIdx);
	glm::vec3 vectorToTarget = targetPos - transform.pos; // vector from the spark to target location
	float distance = glm::length(vectorToTarget); // get the length of this vector to get the distance
	

	calcSteering(ai, controls, transform);

	// ---- THROTTLE ----
	float curvature = ai.angles.at(ai.targetIdx); // curvature 0 = straight, 1 = curve
	float targetSpeed = glm::mix(maxTargetSpeed, 1.0f, curvature);

	
	dbug::log("AI", 0, "CURVE: %.2f, CURRENT SPEED: %.2f, TARGET SPEED: %.2f, LOOK: %d", curvature, spark.speed, targetSpeed, ai.lookAheadSteps);


	if (curvature >= curveAngleThresh && spark.speed > targetSpeed) {
		// angle of curve steeper than threshold: slow speed for upcoming turn
		ai.state = BRAKING;
		dbug::log("AI", 0, "Entity: DRIVING -> BRAKING (dist: %.1f)", distance);
		return;
		
	}
	else if (curvature < 0.12f && spark.currBoost >= 25.f) {
		ai.state = BOOSTING;
		return;
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

	dbug::log("AI", 0, "Entity: dist = %.1f steer=%.2f throttle = %.2f brake = %.2f curr location = (%.1f, %.1f, %.1f)", distance, controls.steering, controls.throttle, controls.brake, transform.pos.x, transform.pos.y, transform.pos.z);
	return;
}

void AIControllerSys::AI_BRAKING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	
	calcSteering(ai, controls, transform);

	// Brake based on difference in current speed and target speed
	float curvature = ai.angles.at(ai.targetIdx);
	float targetSpeed = glm::mix(maxTargetSpeed, 1.0f, curvature);
	
	if (targetSpeed <= 0.0f) {
		// Occasional bug where target speed would end up negative here, causes spark to brake to a stop
		controls.throttle = 1.0f;
		controls.brake = 0.0f;
		ai.state = DRIVING;
		return;
	}

	// amount of throttle/brake to add per unit difference in speed
 	float throttleGain = 0.6f;
	float brakeGain = 0.6f;

	float speedDiff = targetSpeed - spark.speed;
	controls.brake = glm::clamp(-speedDiff * brakeGain, 0.0f, 1.0f);
	
	if (controls.brake > 0.05) 
		controls.throttle = 0.0f; // avoid pressing brake and throttle at same time
	else controls.throttle = glm::clamp(speedDiff * throttleGain, 0.0f, 1.0f);

	dbug::log("AI", 0, "BRAKING: %0.2f", controls.brake);

	if (spark.speed <= targetSpeed)
		ai.state = DRIVING;
	

	// zero out unused controls
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;

	//dbug::log("AI", 0, "Entity: BRAKING dist = %.1f steer=%.2f throttle = %.2f brake = %.2f", distance, controls.steering, controls.throttle, controls.brake);
	return;
}

void AIControllerSys::AI_DRIFTING(AIController& ai, SparkControls& controls, Transform& transform) {
	// TODO: drift logic when drifting is implemented
	return;
}

void AIControllerSys::AI_BOOSTING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	// TODO: add boost logic for AI
	calcSteering(ai, controls, transform);

	controls.throttle = 1.0f;
	controls.brake = 0.0f;
	controls.boost = true;
	dbug::log("AI", 0, "BOOSTING -> BOOST AMT: %.2f", spark.currBoost);

	float curvature = ai.angles.at(ai.targetIdx);
	if (curvature > curveAngleThresh) {
		ai.state = BRAKING;
		controls.boost = false;
		return;
	}
	else if (curvature > 0.12f || spark.currBoost < 25.f) {
		ai.state = DRIVING;
		controls.boost = false;
	}

	return;
}

void AIControllerSys::AI_ATTACKING(AIController& ai, SparkControls& controls, Transform& transform) {
	// TODO: add logic for attacking other players
	return;
}


void AIControllerSys::calcSteering(AIController& ai, SparkControls& controls, Transform& transform) {
	glm::vec3 targetPos = ai.route.at(ai.targetIdx);
	glm::vec3 vectorToTarget = targetPos - transform.pos; // vector from the spark to target location
	float distance = glm::length(vectorToTarget); // get the length of this vector to get the distance


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
	float steerRaw = -(angle / (glm::pi<float>() / 2)) * ai.steeringSharpness;
	controls.steering = glm::clamp(steerRaw, -1.0f, 1.0f);

	return;
}