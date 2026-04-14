#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "world/CurveLoader.h"
#include "world/Clock.cpp"

// FSM states for AI sparks
// NOTE: no longer used (left in for debugging)
enum AIDriveState {
	IDLE = 0,
	DRIVING = 1,
	BRAKING = 2,
	DRIFTING = 3,
	BOOSTING = 4,
	ATTACKING = 5,
	DODGING = 6
};
// Direction enum for labelling sweep/hit direction
enum Direction {
	FWD,
	SIDE_L,
	SIDE_R,
	BACK,
	NONE // no hit
};

// this component tells the controller system that its AI controlled.
struct AIController {
	AIDriveState state = IDLE;

	std::vector<glm::vec3> route{}; // Current route plan for the ai
	std::vector<float> angles{}; // Set of 'curvature' angles for each point in the route
	PathID routeID = pFAST;
	float curveDriftThresh = 0.10f; // minimum angle of turn for spark to drift
	float curveBrakeThresh = 0.50f; // minimum angle of turn for spark to decrease speed. Allows spark to increase speed indefinitely on any path shallower than this.
	float maxTargetSpeed = 50.0f; // Max speed for target speed calculated based on angle of turn (when angle of turn is above threshold)
	float curveBoostThresh = 0.05f; // max angle for ai to consider boosting
	float steeringSharpness = 4.0f; // how aggressively the AI turns. 1.0 = 90 degrees. higher = 'snappier' turns

	// No reason to set these, default values are fine
	int targetIdx = 0; // index of target position
	int currentPosIdx = 0; // approx index current position of spark
	int lookAheadSteps = 8; // how far to look ahead on the track curve
	int logIdx = 0;
	float boostAtkTimer = 0.0f;
	float dodgeTimer = 0.0f;
	float respawnRecoverTimer = 0.0f;
	int lastPosIdx = 0;
	
	bool recoverAttempt = false; // track if a stuck ai has made an attempt to recover yet
	Direction recoverDir = NONE; // each recovery attempt has a recovery direction: FWD, BACK, or NONE if not found yet
	Clock checkProgTimer{2.0, 2.0}; // counts while ai is not moving
	Clock recoverClock{ 1.0, 1.0 }; // amount of time to stay in recover state
	Clock attackCooldown{ 7.0, 7.0 }; // only this long at the beginning of the race. Cooldown shorter after that
};


