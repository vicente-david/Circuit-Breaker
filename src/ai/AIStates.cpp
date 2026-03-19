#include "AIStates.h"

// Note: these all do the same thing right now.
/*
* Drive defensively: prioritize dodging and recovering HP
*/
void DefenseState::run(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	//dbug::log("AI", 0, " !!!!! in defense state");

	if (ai.state == DRIVING)
		AI_DRIVING(ai, controls, transform, spark);
	else if (ai.state == BRAKING)
		AI_BRAKING(ai, controls, transform, spark);
	else if (ai.state == DRIFTING)
		AI_DRIFTING(ai, controls, transform, spark);
	else if (ai.state == BOOSTING)
		AI_BOOSTING(ai, controls, transform, spark);
	else if (ai.state == ATTACKING)
		AI_ATTACKING(ai, controls, transform);
}

/*
* Drive offensively to overtake other players: prioritize speed and attacking others
*/
void OvertakeState::run(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, PxRigidBody* body) {

	// TODO: set appropriate path to follow


	dbug::log("AI", 0, "!!!!! in overtake state");
	OvertakeState::detect(ai, controls, transform, spark, body);

	if (ai.state == DRIVING)
		AI_DRIVING(ai, controls, transform, spark);
	else if (ai.state == BRAKING)
		AI_BRAKING(ai, controls, transform, spark);
	else if (ai.state == DRIFTING)
		AI_DRIFTING(ai, controls, transform, spark);
	else if (ai.state == BOOSTING)
		AI_BOOSTING(ai, controls, transform, spark);
	else if (ai.state == ATTACKING)
		AI_ATTACKING(ai, controls, transform);
}

/*
* Drive to maintain a lead: take less risks to maintain in the lead
*/
void MaintainState::run(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {

	//dbug::log("AI", 0, "!!!!! in maintain state");
	if (ai.state == DRIVING)
		AI_DRIVING(ai, controls, transform, spark);
	else if (ai.state == BRAKING)
		AI_BRAKING(ai, controls, transform, spark);
	else if (ai.state == DRIFTING)
		AI_DRIFTING(ai, controls, transform, spark);
	else if (ai.state == BOOSTING)
		AI_BOOSTING(ai, controls, transform, spark);
	else if (ai.state == ATTACKING)
		AI_ATTACKING(ai, controls, transform);
}

// Detect if there is another player spark in line of sight
void OvertakeState::detect(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, PxRigidBody* body) {
	PxScene* scene = body->getScene();
	
	PxSweepBuffer hitInfo;
	PxVec3 forwardDir(transform.forwardD.x, transform.forwardD.y, transform.forwardD.z); // sweep in front facing direction
	PxBoxGeometry sweepBox(1.f, 0.5f, 0.5f); // geometry to sweep
	PxTransform initPose = body->getGlobalPose();
	initPose.p += forwardDir.getNormalized() * 2.f; // set initial pose to be a bit in front of the spark

	const PxHitFlags outFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filter = PxQueryFilterData(PxQueryFlag::eDYNAMIC); // NOTE: detects any dynamic actor. Could not get it to work otherwise.
	//filter.flags |= PxQueryFlag::eANY_HIT;
	//filter.data.word0 = COLLISION_FLAG_CHASSIS;
	bool status = scene->sweep(sweepBox, initPose, forwardDir.getNormalized(), 75.f, hitInfo, outFlags, filter);
	
	// Check if hit returned true and if the hit was not itself
	if (status && body->getInternalActorIndex() != hitInfo.block.actor->getInternalActorIndex()) {
		std::cout << "hit?" << std::endl;
	}
}

// DRIVING STATES ==============================================================================================================

void AIState::AI_DRIVING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	// Get target position and calculate steering
	glm::vec3 targetPos = ai.route.at(ai.targetIdx);
	glm::vec3 vectorToTarget = targetPos - transform.pos;
	float distance = glm::length(vectorToTarget);
	calcSteering(ai, controls, transform, spark);


	if (abs(controls.steering) > 1.0f) {
		// spark most likely has lost steering control
		ai.state = BRAKING;
		return;
	}

	// Speed and curve of target and current positions.
	// target speed is calculated based on how much curve is ahead (sharper curve = slower speed)
	float curvature = ai.angles.at(ai.targetIdx); // curvature 0 = straight, 1 = curve
	float targetSpeed = glm::mix(ai.maxTargetSpeed, 1.0f, curvature);
	float curveIn = ai.angles.at(ai.currentPosIdx);
	dbug::log("AI", 0, "[DRIVING] CURVE: %.2f, CURRENT SPEED: %.2f, TARGET SPEED: %.2f, LOOK: %d, BOOST: %.2f, HP: %.2f", curvature, spark.speed, targetSpeed, ai.lookAheadSteps, spark.currBoost, spark.health);


	// Boost for speed if along straight path, not running out of boost, not facing downwards and not when steering sharply
	if (curvature < ai.curveBoostThresh && spark.currBoost >= 0.f && transform.pos.y > -0.08f && abs(controls.steering) < 0.8f) {
		ai.state = BOOSTING;
		return;
	}
	// Brake if there is a sharp turn ahead and the spark is travelling faster than its target speed
	else if (curvature >= ai.curveBrakeThresh && (spark.speed - targetSpeed) > 15.f && curveIn < curvature) {
		ai.state = BRAKING;
		return;
	}
	// If the curve ahead is large enough, attempt drifting
	else if (curvature >= ai.curveDriftThresh) {
		ai.state = DRIFTING;
		return;

	}
	// Otherwise, continue in driving state
	else {
		if (spark.speed <= targetSpeed)
			controls.throttle = 1.0f;
		else
			controls.throttle = 0.0f;
		controls.brake = 0.0f;
	}

	// zero out the unused controls
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;

	return;
}

void AIState::AI_BRAKING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {

	calcSteering(ai, controls, transform, spark);

	// Brake based on difference in current speed and target speed
	float curvature = ai.angles.at(ai.targetIdx);
	float targetSpeed = glm::mix(ai.maxTargetSpeed, 1.0f, curvature);

	if (targetSpeed <= 0.0f) {
		// Occasional bug where target speed would end up negative here, causes spark to brake to a stop
		controls.throttle = 1.0f;
		controls.brake = 0.0f;
		ai.state = DRIVING;
		return;
	}

	// amount of throttle/brake to add per unit difference in speed
	float throttleGain = 0.3f;
	float brakeGain = 0.2f;

	float speedDiff = targetSpeed - spark.speed;

	if (speedDiff > 20.f) { // if speed differenece is very high, do not slam on brakes
		controls.brake = 0.f;
		controls.throttle = 0.f;
	}
	else
		controls.brake = glm::clamp(-speedDiff * brakeGain, 0.0f, 1.0f);

	if (controls.brake > 0.05)
		controls.throttle = 0.0f; // avoid pressing brake and throttle at same time
	else controls.throttle = glm::clamp(speedDiff * throttleGain, 0.0f, 1.0f);

	dbug::log("AI", 0, "[BRAKING] Brake: %0.2f", controls.brake);

	if (spark.speed <= targetSpeed) {
		ai.state = DRIVING;
	}

	// zero out unused controls
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;

	//dbug::log("AI", 0, "Entity: BRAKING dist = %.1f steer=%.2f throttle = %.2f brake = %.2f", distance, controls.steering, controls.throttle, controls.brake);
	return;
}

void AIState::AI_DRIFTING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	calcSteering(ai, controls, transform, spark);

	float curvature = ai.angles.at(ai.targetIdx);
	float targetSpeed = glm::mix(ai.maxTargetSpeed, 1.0f, curvature);

	// amount of throttle to add per unit difference in speed
	float throttleGain = 0.5f;
	float speedDiff = targetSpeed - spark.speed;
	controls.handbrake = true; // note: handbrake isn't actually a brake, should be held down to drift.
	controls.throttle = 1.0f; //glm::clamp(speedDiff * throttleGain, 0.0f, 1.0f);

	dbug::log("AI", 0, "[DRIFTING] THROTTLE: %.2f, CURVE: %.2f, \n\tBOOST: %.2f, HP: %.2f", controls.throttle, curvature, spark.currBoost, spark.health);

	// Let go of drift when the curve flattens out
	if (curvature < ai.curveDriftThresh) {
		controls.handbrake = false;
		ai.state = DRIVING;
	}

	// zero out unused controls
	controls.reverse = 0.0f;
	controls.boost = false;
	controls.shimmyL = false;
	controls.shimmyR = false;

	return;
}

void AIState::AI_BOOSTING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	calcSteering(ai, controls, transform, spark);

	controls.throttle = 1.0f;
	controls.brake = 0.0f;
	controls.boost = true;
	dbug::log("AI", 0, "BOOSTING -> BOOST: %.2f, HEALTH: %.2f", spark.currBoost, spark.health);

	float curvature = ai.angles.at(ai.targetIdx);
	if (curvature > ai.curveBrakeThresh) {
		ai.state = BRAKING; // If curvature of lookahead is passed threshold, go straight to braking
		controls.boost = false;
		return;
	}
	else if (curvature >= ai.curveBoostThresh || spark.currBoost <= 0.0f) {
		ai.state = DRIVING;
		controls.boost = false;
	}

	return;
}

void AIState::AI_ATTACKING(AIController& ai, SparkControls& controls, Transform& transform) {
	// TODO: add logic for attacking other players



	return;
}


// Steering calculations used in many of the above
void AIState::calcSteering(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	glm::vec3 targetPos = ai.route.at(ai.targetIdx);
	glm::vec3 vectorToTarget = targetPos - transform.pos; // vector from the spark to target location
	vectorToTarget.y = 0.0f;
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
	dbug::log("AI", 0, "ANGLE: %.3f", angle);

	// map the angle to [-1, 1] steering
	// lock before sharpness multiplier
	float steerRaw = -(angle / (glm::pi<float>() / 2)) * ai.steeringSharpness;
	controls.steering = glm::clamp(steerRaw, -1.0f, 1.0f);

	return;
}