#pragma once
#include <glm/glm.hpp>
#include <vector>

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

// this component tells the controller system that its AI controlled.
struct AIController {
	AIDriveState state = IDLE;

	std::vector<glm::vec3> route{}; // Current route plan for the ai
	std::vector<float> angles{}; // Set of 'curvature' angles for each point in the route
	float curveDriftThresh = 0.10f; // minimum angle of turn for spark to drift
	float curveBrakeThresh = 0.30f; // minimum angle of turn for spark to decrease speed. Allows spark to increase speed indefinitely on any path shallower than this.
	float maxTargetSpeed = 30.0f; // Max speed for target speed calculated based on angle of turn (when angle of turn is above threshold)
	float curveBoostThresh = 0.05f; // max angle for ai to consider boosting
	float steeringSharpness = 4.0f; // how aggressively the AI turns. 1.0 = 90 degrees. higher = 'snappier' turns

	// No reason to set these, default values are fine
	int targetIdx = 0; // index of target position
	int currentPosIdx = 0; // approx index current position of spark
	int lookAheadSteps = 8; // how far to look ahead on the track curve
	float boostAtkTimer = 0.0f;
};

