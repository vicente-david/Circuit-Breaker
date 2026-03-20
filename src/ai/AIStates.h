#pragma once

#include "GameState.h"
#include "ai/AISparkComponents.h"
#include "vehicles/SparkComponents.h"
#include "ecs/System.h"
#include "world/CurveLoader.h"

// Direction enum for labelling sweep/hit direction
enum Direction {
	FWD,
	LEFT,
	RIGHT,
	NONE // no hit
};

/*
* Base AI state class
* Not used directly by the AIControllerSys.
* Contains all the 'low-level' driving states that each high-level state can use (with the option of "overriding" (not really because they're all static) behaviour).
*/
class AIState {

public: 
	static void AI_DRIVING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
	static void AI_BRAKING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
	static void AI_DRIFTING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
	static void AI_BOOSTING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
	static void AI_ATTACKING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, std::pair<Direction, glm::vec3>& sweepResult);
	static void AI_DODGING(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, std::pair<Direction, glm::vec3>& sweepResult);

	static std::pair<bool, glm::vec3> lookFwd(Transform& transform, PxRigidBody* body);
	static std::pair<bool, glm::vec3> lookSide(Transform& transform, PxRigidBody* body, Direction& dir);

private:
	static void calcSteering(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, glm::vec3& targetPos);

};

/*
* Derived state classes.
* These are high-level states used by the ai sparks to determine the 'style' of driving to use.
*/
class DefenseState : public AIState {

public:
	static void run(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, PxRigidBody* body);
	static std::pair<Direction, glm::vec3> detect(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, PxRigidBody* body);

};

class OvertakeState : public AIState {

public:
	static void run(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, PxRigidBody* body);
	static std::pair<Direction, glm::vec3> detect(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, PxRigidBody* body);
};

class MaintainState : public AIState {

public:
	static void run(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark);
};