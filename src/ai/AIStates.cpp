#include "AIStates.h"


/*
* ===== DEFENSE STATE =============================================================================================================================================
* prioritize dodging and recovering HP
*/
void DefenseState::run(AIDriveContext& ctx) {
	
	std::pair<Direction, glm::vec3> sweepResult = DefenseState::detect(ctx); // Line of sight sweep
	if (sweepResult.first != NONE) {
		auto next = std::make_unique<S_Dodging>();
		next->sweepResult = sweepResult;
		currentState = std::move(next);
	}

	auto next = currentState->update(ctx);
	if (next) {
		// if the returned pointer was not nullptr (points to a new state), change states
		currentState = std::move(next);
		currentState->enter(ctx);
	}
	
}

// Detect if there is another player spark in line of sight
std::pair<Direction, glm::vec3> DefenseState::detect(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& transform = ctx.transform;
	auto& body = ctx.body;
	std::pair<Direction, glm::vec3> result{ NONE, glm::vec3(0.f) };

	Direction dir = LEFT;
	std::pair<bool, glm::vec3> resultSide = AIState::lookSide(transform, body, dir);
	if (resultSide.first) {
		ai.state = DODGING;
		result = { LEFT, resultSide.second };
		std::cout << "hit left" << std::endl;
		return result;
	}

	dir = RIGHT;
	resultSide = AIState::lookSide(transform, body, dir);
	if (resultSide.first) {
		ai.state = DODGING;
		result = { RIGHT, resultSide.second };
		std::cout << "hit right" << std::endl;
		return result;
	}

	// No sweep returned with a hit
	if (ai.state == DODGING) {
		ai.state = DRIVING; // if ai in dodging state but can no longer see an enemy, switch to driving state
	}

	return result; // return direction and position of hit point.
}


/*
* ===== OVERTAKE STATE =============================================================================================================================================
* overtake other players: prioritize speed and attacking others
*/
void OvertakeState::run(AIDriveContext& ctx) {

	// TODO: set appropriate path to follow


	std::pair<Direction, glm::vec3> sweepResult = OvertakeState::detect(ctx); // Line of sight sweep

	/*if (ai.state == DRIVING)
		AI_DRIVING(ai, controls, transform, spark);
	else if (ai.state == BRAKING)
		AI_BRAKING(ai, controls, transform, spark);
	else if (ai.state == DRIFTING)
		AI_DRIFTING(ai, controls, transform, spark);
	else if (ai.state == BOOSTING)
		AI_BOOSTING(ai, controls, transform, spark);
	else if (ai.state == ATTACKING)
		AI_ATTACKING(ai, controls, transform, spark, sweepResult);*/
}

// Detect if there is another player spark in line of sight
std::pair<Direction, glm::vec3> OvertakeState::detect(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& transform = ctx.transform;
	auto& spark = ctx.spark;
	auto& body = ctx.body;
	std::pair<Direction, glm::vec3> result{ NONE, glm::vec3(0.f) };

	// Only check forward direction if spark has boost
	if (spark.currBoost > 0.0f) {
		std::pair<bool, glm::vec3> resultFwd = AIState::lookFwd(transform, body);

		// Check if hit returned true and if the hit was not itself
		if (resultFwd.first) {
			ai.state = ATTACKING;
			result = { FWD, resultFwd.second };
			return result;
		}
	}

	// Test side directions (left and right)
	else {
		Direction dir = LEFT;
		std::pair<bool, glm::vec3> resultSide = AIState::lookSide(transform, body, dir);
		if (resultSide.first) {
			ai.state = ATTACKING;
			result = { LEFT, resultSide.second };
			std::cout << "hit left" << std::endl;
			return result;
		}

		dir = RIGHT;
		resultSide = AIState::lookSide(transform, body, dir);
		if (resultSide.first) {
			ai.state = ATTACKING;
			result = { RIGHT, resultSide.second };
			std::cout << "hit right" << std::endl;
			return result;
		}

	}

	// No sweep returned with a hit
	if (ai.state == ATTACKING) {
		ai.state = DRIVING; // if ai in attacking state but can no longer see an enemy, switch to driving state
		//controls.boost = false; // ensure false
	}

	return result; // return direction and position of hit point.
}

/*
* ===== MAINTAIN STATE =============================================================================================================================================
* Drive to maintain a lead: take less risks to maintain in the lead
*/
void MaintainState::run(AIDriveContext& ctx) {

	//dbug::log("AI", 0, "!!!!! in maintain state");
	/*if (ai.state == DRIVING)
		AI_DRIVING(ai, controls, transform, spark);
	else if (ai.state == BRAKING)
		AI_BRAKING(ai, controls, transform, spark);
	else if (ai.state == DRIFTING)
		AI_DRIFTING(ai, controls, transform, spark);
	else if (ai.state == BOOSTING)
		AI_BOOSTING(ai, controls, transform, spark);*/
	
}







// =========== DRIVING STATES =============================================================================================================================================================

std::unique_ptr<IDriveState> S_Driving::update(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& controls = ctx.controls;
	auto& transform = ctx.transform;
	auto& spark = ctx.spark;
	AIState::calcSteering(ai, controls, transform, spark, ai.route.at(ai.targetIdx));

	if (abs(controls.steering) > 1.0f) {
		// spark most likely has lost steering control
		ai.state = BRAKING;
		return std::make_unique<S_Braking>();
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
		return std::make_unique<S_Boosting>();
	}
	// Brake if there is a sharp turn ahead and the spark is travelling faster than its target speed
	else if (curvature >= ai.curveBrakeThresh && (spark.speed - targetSpeed) > 15.f && curveIn < curvature) {
		ai.state = BRAKING;
		return std::make_unique<S_Braking>();
	}
	// If the curve ahead is large enough, attempt drifting
	else if (curvature >= ai.curveDriftThresh) {
		ai.state = DRIFTING;
		return std::make_unique<S_Drifting>();

	}
	// Otherwise, continue in driving state
	else {
		if (spark.speed <= targetSpeed)
			controls.throttle = 1.0f;
		else
			controls.throttle = 0.0f;
		controls.brake = 0.0f;
	}

	return nullptr;
}

std::unique_ptr<IDriveState> S_Braking::update(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& controls = ctx.controls;
	auto& transform = ctx.transform;
	auto& spark = ctx.spark;
	AIState::calcSteering(ai, controls, transform, spark, ai.route.at(ai.targetIdx));

	// Brake based on difference in current speed and target speed
	float curvature = ai.angles.at(ai.targetIdx);
	float targetSpeed = glm::mix(ai.maxTargetSpeed, 1.0f, curvature);

	if (targetSpeed <= 0.0f) {
		// Occasional bug where target speed would end up negative here, causes spark to brake to a stop
		controls.throttle = 1.0f;
		controls.brake = 0.0f;
		ai.state = DRIVING;
		return std::make_unique<S_Driving>();
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
		return std::make_unique<S_Driving>();
	}

	return nullptr;
}

std::unique_ptr<IDriveState> S_Drifting::update(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& controls = ctx.controls;
	auto& transform = ctx.transform;
	auto& spark = ctx.spark;
	AIState::calcSteering(ai, controls, transform, spark, ai.route.at(ai.targetIdx));

	float curvature = ai.angles.at(ai.targetIdx);
	float targetSpeed = glm::mix(ai.maxTargetSpeed, 1.0f, curvature);

	controls.handbrake = true; // note: handbrake isn't actually a brake, should be held down to drift.
	controls.throttle = 1.0f; 

	dbug::log("AI", 0, "[DRIFTING] THROTTLE: %.2f, CURVE: %.2f, \n\tBOOST: %.2f, HP: %.2f", controls.throttle, curvature, spark.currBoost, spark.health);

	// Let go of drift when the curve flattens out
	if (curvature < ai.curveDriftThresh) {
		controls.handbrake = false;
		ai.state = DRIVING;
		return std::make_unique<S_Driving>();
	}

	return nullptr;
}

std::unique_ptr<IDriveState> S_Boosting::update(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& controls = ctx.controls;
	auto& transform = ctx.transform;
	auto& spark = ctx.spark;
	AIState::calcSteering(ai, controls, transform, spark, ai.route.at(ai.targetIdx));

	dbug::log("AI", 0, "BOOSTING -> BOOST: %.2f, HEALTH: %.2f", spark.currBoost, spark.health);

	float curvature = ai.angles.at(ai.targetIdx);
	if (curvature > ai.curveBrakeThresh) {
		ai.state = BRAKING; // If curvature of lookahead is passed threshold, go straight to braking
		controls.boost = false;
		return std::make_unique<S_Braking>();
	}
	else if (curvature >= ai.curveBoostThresh || spark.currBoost <= 0.0f) {
		ai.state = DRIVING;
		controls.boost = false;
		return std::make_unique<S_Driving>();
	}

	return nullptr;
}

std::unique_ptr<IDriveState> S_Attacking::update(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& controls = ctx.controls;
	auto& transform = ctx.transform;
	auto& spark = ctx.spark;

	// Sanity check that a hit was actually detected
	if (sweepResult.first == NONE) {
		controls.boost = false;
		controls.shimmyL = false;
		controls.shimmyR = false;
		return std::make_unique<S_Driving>();;
	}

	if (sweepResult.first == FWD) {
		// Boost attack
		if (spark.currBoost > 0.0f) {
			AIState::calcSteering(ai, controls, transform, spark, sweepResult.second); // steer to position of collision point

			float distTo = glm::length(sweepResult.second - transform.pos);

			// Boost towards target
			controls.throttle = 1.0f;
			controls.brake = 0.0f;
			controls.boost = true;
			dbug::log("AI", 0, "[ATTACKING] -> DIST TO TARGET: %.2f, BOOST: %.2f, HEALTH: %.2f", distTo, spark.currBoost, spark.health);

		}

		if (spark.currBoost <= 0.0f) {
			controls.boost = false;
			ai.state = DRIVING; // out of boost: exit attack
			return std::make_unique<S_Driving>();
		}
	}

	else {
		// Shimmy attack
		if (sweepResult.first == LEFT) {
			controls.shimmyL = true;
			dbug::log("AI", 0, "[ATTACKING] -> SHIMMY LEFT");
		}
		else if (sweepResult.first == RIGHT) {
			controls.shimmyR = true;
			dbug::log("AI", 0, "[ATTACKING] -> SHIMMY RIGHT");
		}
	}


	return nullptr;
}

std::unique_ptr<IDriveState> S_Dodging::update(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& controls = ctx.controls;
	auto& transform = ctx.transform;
	auto& spark = ctx.spark;

	// Check that a hit is still detected
	if (sweepResult.first == NONE) {
		controls.boost = false;
		controls.shimmyL = false;
		controls.shimmyR = false;
		return std::make_unique<S_Driving>();
	}

	glm::vec3 vecToOpponent = transform.pos - sweepResult.second;
	float angleBetween = glm::acos(glm::dot(glm::normalize(transform.forwardD), glm::normalize(vecToOpponent))); // angle between the forward direction and direction to the detected opponent

	std::cout << "angle btwn: " << angleBetween << std::endl;
	if (angleBetween > glm::radians(90.f) && spark.currBoost > 0.0f) {
		// opponent is slightly behind: try to boost away
		glm::quat rotateAway;
		if (sweepResult.first == LEFT) {
			rotateAway = glm::angleAxis(glm::radians(10.f), glm::vec3(0.f, 1.f, 0.f));
		}
		else
			rotateAway = glm::angleAxis(glm::radians(-10.f), glm::vec3(0.f, 1.f, 0.f));

		glm::vec3 escapeDir = ai.route.at(ai.targetIdx) - transform.pos; // vector between spark and target index
		escapeDir = rotateAway * escapeDir;
		glm::vec3 target = transform.pos + escapeDir; // Get lookahead position rotated away from detected opponent


		AIState::calcSteering(ai, controls, transform, spark, target); // Calculate steering towards escape direction
		controls.boost = true;
		dbug::log("AI", 0, "[DODGING] -> BOOST AWAY");
	}
	else if (spark.currBoost <= 0.0f) {
		ai.state = DRIVING;
		controls.boost = false;
		return std::make_unique<S_Driving>();
	}


	return nullptr;
}
















void AIState::AI_DRIVING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark) {
	
	calcSteering(ai, controls, transform, spark, ai.route.at(ai.targetIdx));


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

	calcSteering(ai, controls, transform, spark, ai.route.at(ai.targetIdx));

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
	calcSteering(ai, controls, transform, spark, ai.route.at(ai.targetIdx));

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
	calcSteering(ai, controls, transform, spark, ai.route.at(ai.targetIdx));

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

void AIState::AI_ATTACKING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, std::pair<Direction, glm::vec3>& sweepResult) {

	// Sanity check that a hit was actually detected
	if (sweepResult.first == NONE) {
		controls.boost = false;
		controls.shimmyL = false;
		controls.shimmyR = false;
		return;
	}

	if (sweepResult.first == FWD) {
		// Boost attack
		if (spark.currBoost > 0.0f) {
			calcSteering(ai, controls, transform, spark, sweepResult.second); // steer to position of collision point

			float distTo = glm::length(sweepResult.second - transform.pos);

			// Boost towards target
			controls.throttle = 1.0f;
			controls.brake = 0.0f;
			controls.boost = true;
			dbug::log("AI", 0, "[ATTACKING] -> DIST TO TARGET: %.2f, BOOST: %.2f, HEALTH: %.2f", distTo, spark.currBoost, spark.health);

		}

		if (spark.currBoost <= 0.0f) {
			controls.boost = false;
			ai.state = DRIVING; // out of boost: exit attack
		}
	}
	
	else {
		// Shimmy attack
		if (sweepResult.first == LEFT) {
			controls.shimmyL = true;
			dbug::log("AI", 0, "[ATTACKING] -> SHIMMY LEFT");
		}
		else if (sweepResult.first == RIGHT) {
			controls.shimmyR = true;
			dbug::log("AI", 0, "[ATTACKING] -> SHIMMY RIGHT");
		}
	}
	
	

	return;
}

void AIState::AI_DODGING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, std::pair<Direction, glm::vec3>& sweepResult) {

	// Sanity check that a hit was actually detected
	if (sweepResult.first == NONE) {
		controls.boost = false;
		controls.shimmyL = false;
		controls.shimmyR = false;
		return;
	}
	
	glm::vec3 vecToOpponent = transform.pos - sweepResult.second;
	float angleBetween = glm::acos(glm::dot(glm::normalize(transform.forwardD), glm::normalize(vecToOpponent))); // angle between the forward direction and direction to the detected opponent
	
	std::cout << "angle btwn: " << angleBetween << std::endl;
	if (angleBetween > glm::radians(90.f) && spark.currBoost > 0.0f) {
		// opponent is slightly behind: try to boost away
		glm::quat rotateAway;
		if (sweepResult.first == LEFT) {
			rotateAway = glm::angleAxis(glm::radians(10.f), glm::vec3(0.f, 1.f, 0.f));
		}
		else
			rotateAway = glm::angleAxis(glm::radians(-10.f), glm::vec3(0.f, 1.f, 0.f));

		glm::vec3 escapeDir = ai.route.at(ai.targetIdx) - transform.pos; // vector between spark and target index
		escapeDir = rotateAway * escapeDir;
		glm::vec3 target = transform.pos + escapeDir; // Get lookahead position rotated away from detected opponent
		
		
		calcSteering(ai, controls, transform, spark, target); // Calculate steering towards escape direction
		controls.boost = true;
		dbug::log("AI", 0, "[DODGING] -> BOOST AWAY");
	}
	else if (spark.currBoost <= 0.0f) {
		ai.state = DRIVING;
		controls.boost = false;
	}
	

	return;

}


// ====== HELPER FUNCTIONS ===========================================================================================================================================================

// Perform forward sweep to detect opponents
std::pair<bool, glm::vec3> AIState::lookFwd(Transform& transform, PxRigidBody* body) {
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
	bool status = scene->sweep(sweepBox, initPose, forwardDir.getNormalized(), 100.f, hitInfo, outFlags, filter);

	// Check if hit returned true and if the hit was not itself
	if (status && body->getInternalActorIndex() != hitInfo.block.actor->getInternalActorIndex()) {
		std::cout << "hit fwd" << std::endl;

		PxVec3 t = hitInfo.block.position;
		glm::vec3 target(t.x, t.y, t.z);

		return { true, target };
	}
	else
		return { false, glm::vec3(0.f) };

}

// Perform sideways sweep to detect opponents
std::pair<bool, glm::vec3> AIState::lookSide(Transform& transform, PxRigidBody* body, Direction& dir) {
	PxScene* scene = body->getScene();

	PxSweepBuffer hitInfo;

	// Locally rotate forward direction by -90(L) or 90(R) degrees
	glm::quat rotate;
	if (dir == LEFT) {
		rotate = glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
	}
	else {
		rotate = glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
	}
	glm::vec3 d = rotate * transform.forwardD;
	PxVec3 direction(d.x, d.y, d.z);

	PxBoxGeometry sweepBox(.2f, 0.25f, 0.5f); // geometry to sweep
	PxTransform initPose = body->getGlobalPose();
	initPose.p += direction.getNormalized() * 2.f; // set initial pose to be a bit away from spark

	const PxHitFlags outFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filter = PxQueryFilterData(PxQueryFlag::eDYNAMIC);

	bool status = scene->sweep(sweepBox, initPose, direction.getNormalized(), 50.f, hitInfo, outFlags, filter);

	// Check if hit returned true and if the hit was not itself
	if (status && body->getInternalActorIndex() != hitInfo.block.actor->getInternalActorIndex()) {
		std::cout << "hit side" << std::endl;

		PxVec3 t = hitInfo.block.position;
		glm::vec3 target(t.x, t.y, t.z);

		return { true, target };
	}
	else
		return { false, glm::vec3(0.f) };
}


// Steering calculations used in many of the above
void AIState::calcSteering(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, glm::vec3& targetPos) {
	glm::vec3 vectorToTarget = targetPos - transform.pos; // vector from the spark to target location
	vectorToTarget.y = 0.0f;
	float distance = glm::length(vectorToTarget); // get the length of this vector to get the distance


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