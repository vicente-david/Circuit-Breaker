#pragma once

#include <glm/glm.hpp>

enum AI_STATES {
	IDLE = 0,
	ACCELERATING = 1,
	BRAKING = 2,
	TURNING = 3,
	DRIFTING = 4,
	BOOSTING = 5,
	ATTACKING = 6
};
class AIController() {
	public:
		AIController(); // constructor
		updateBehaviour(); // update AI state

		// state functionalities
		AI_Idle();
		AI_Accelerating();
		AI_Braking();
		AI_Turning();
		AI_Drifting();
		AI_Boosting();
		AI_Attacking();

		// AI agent variables
		glm::vec3 currentPos;
		float currentHealth;
		float currentBoost;

	private:
		int state;
}