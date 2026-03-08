#pragma once
#include <glm/glm.hpp>

// FSM states for AI sparks
enum AIState {
	IDLE = 0,
	DRIVING = 1,
	BRAKING = 2,
	DRIFTING = 3,
	BOOSTING = 4,
	ATTACKING = 5
};

// this component tells the controller system that its AI controlled.
struct AIController {
	AIState state = IDLE;

	std::vector<glm::vec3> route{}; // Current route plan for the ai
	std::vector<float> angles{}; // Set of 'curvature' angles for each point in the route
	float curveBrakeThresh = 0.10f; // minimum angle of turn for spark to decrease speed. Allows spark to increase speed indefinitely on any path shallower than this.
	float maxTargetSpeed = 20.0f; // Max speed for target speed calculated based on angle of turn (when angle of turn is above threshold)
	float curveBoostThresh = 0.03f; // max angle for ai to consider boosting
	float steeringSharpness = 4.0f; // how aggressively the AI turns. 1.0 = 90 degrees. higher = 'snappier' turns

	// No reason to set these, default values are fine
	int targetIdx = 0; // index of target position
	int currentPosIdx = 0; // approx index current position of spark
	int lookAheadSteps = 8; // how far to look ahead on the track curve
	
};

