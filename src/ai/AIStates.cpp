#include "AIStates.h"

using namespace AIHelpers;

/*
* ===== DEFENSE STATE =============================================================================================================================================
* prioritize dodging and recovering HP
*/
void DefenseState::run(AIDriveContext& ctx) {
	dbug::log("AI", 1, "********** AI: %s Defense ***********", ctx.spark.mVehicleName.c_str());
	
	// Check if a route/curve change is needed
	checkRoute(ctx.ai);
	
	// Line of sight sweep: detect other players and change state accordingly
	std::pair<Direction, glm::vec3> sweepResult = DefenseState::detect(ctx);
	if (sweepResult.first != NONE) {
		auto next = std::make_unique<S_Dodging>();
		next->sweepResult = sweepResult;
		currentState = std::move(next);
	}
	// run state update function
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

	Direction dir = SIDE_L;
	std::pair<bool, glm::vec3> resultSide = lookSide(transform, body, dir);
	if (resultSide.first) {
		ai.state = DODGING;
		result = { SIDE_L, resultSide.second };
		return result;
	}

	dir = SIDE_R;
	resultSide = lookSide(transform, body, dir);
	if (resultSide.first) {
		ai.state = DODGING;
		result = { SIDE_R, resultSide.second };
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
	dbug::log("AI", 1, "********** AI: %s Overtake ***********", ctx.spark.mVehicleName.c_str());

	// give ai the proper path
	checkRoute(ctx.ai);

	std::pair<Direction, glm::vec3> sweepResult = OvertakeState::detect(ctx); // Line of sight sweep
	if (sweepResult.first != NONE) {
		auto next = std::make_unique<S_Attacking>();
		next->sweepResult = sweepResult;
		currentState = std::move(next);
	}

	// run state update function
	auto next = currentState->update(ctx);

	if (next) {
		// if the returned pointer was not nullptr (points to a new state), change states
		currentState = std::move(next);
		currentState->enter(ctx);
	}

}


// Detect if there is another player spark in line of sight
std::pair<Direction, glm::vec3> OvertakeState::detect(AIDriveContext& ctx) {
	auto& ai = ctx.ai;
	auto& transform = ctx.transform;
	auto& spark = ctx.spark;
	auto& body = ctx.body;
	std::pair<Direction, glm::vec3> result{ NONE, glm::vec3(0.f) };


	// Test side directions (left and right)
	Direction dir = SIDE_L;
	std::pair<bool, glm::vec3> resultSide = lookSide(transform, body, dir);
	if (resultSide.first) {
		ai.state = ATTACKING;
		result = { SIDE_L, resultSide.second };
		return result;
	}
	dir = SIDE_R;
	resultSide = lookSide(transform, body, dir);
	if (resultSide.first) {
		ai.state = ATTACKING;
		result = { SIDE_R, resultSide.second };
		return result;
	}
	// Only check forward direction if spark has boost
	if (spark.boost > 0.0f) {
		std::pair<bool, glm::vec3> resultFwd = lookFwd(transform, body);
		// Check if hit returned true and if the hit was not itself
		if (resultFwd.first) {
			ai.state = ATTACKING;
			result = { FWD, resultFwd.second };
			return result;
		}
	}


	// No sweep returned with a hit
	ai.state = DRIVING; // if ai in attacking state but can no longer see an enemy, switch to driving state
	
	return { NONE, glm::vec3(0.f) };
	
}



/*
* ===== MAINTAIN STATE =============================================================================================================================================
* Drive to maintain a lead: take less risks to maintain in the lead
*/
void MaintainState::run(AIDriveContext& ctx) {
	dbug::log("AI", 1, "********** AI: %s Maintain ***********", ctx.spark.mVehicleName.c_str());

	checkRoute(ctx.ai);

	// run state update function
	auto next = currentState->update(ctx);

	if (next) {
		// if the returned pointer was not nullptr (points to a new state), change states
		currentState = std::move(next);
		currentState->enter(ctx);
	}
	
}

void AIState::checkRoute(AIController& ai) {

	// Check if a route/curve change is needed
	if (ai.routeID != AIState::path->id) {
		int posIdx = ai.currentPosIdx;

		// check if the ai can switch routes (curve to follow)
		// get the distance between the current point the ai is at on its current route and the point on the desired route at the same index.
		// * for this to work both curves must have the same number of points and line up on singular portions of the track
		float dist = glm::length(ai.route.at(posIdx) - AIState::path->curvePoints.at(posIdx));
		if (dist < 1.f) {
			// if the distance is within a small enough range, change to the new path
			ai.route = AIState::path->curvePoints;
			ai.angles = AIState::path->curvatures;
			ai.routeID = AIState::path->id;
			dbug::log("AIPATH", 1, "Switching to path %d", AIState::path->id);
		}
		else dbug::log("AIPATH", 1, "Path switch refused");
	}
}




