#pragma once

#include "GameState.h"
#include "ai/AISparkComponents.h"
#include "vehicles/SparkComponents.h"
#include "ecs/System.h"
#include "world/CurveLoader.h"
#include "IDriveState.h"

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

	virtual void run(AIDriveContext& ctx) {};
	virtual std::pair<Direction, glm::vec3> detect(AIDriveContext& ctx) { return { NONE, glm::vec3(0.f) }; };

	static std::pair<bool, glm::vec3> lookFwd(Transform& transform, PxRigidBody* body);
	static std::pair<bool, glm::vec3> lookSide(Transform& transform, PxRigidBody* body, Direction& dir);
	static void calcSteering(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, glm::vec3& targetPos);
	
private:

};


class S_Driving : public IDriveState {
public:
	void enter(AIDriveContext& ctx) override {
		// Zero out controls unused in this state
		ctx.controls.reverse = 0.0f;
		ctx.controls.brake = 0.0f;
		ctx.controls.boost = false;
		ctx.controls.shimmyL = false;
		ctx.controls.shimmyR = false;
	}
	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
};

class S_Braking : public IDriveState {
public:
	void enter(AIDriveContext& ctx) override {
		ctx.controls.reverse = 0.0f;
		ctx.controls.boost = false;
		ctx.controls.shimmyL = false;
		ctx.controls.shimmyR = false;
	}
	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
};

class S_Drifting : public IDriveState {
public:
	void enter(AIDriveContext& ctx) override {
		ctx.controls.reverse = 0.0f;
		ctx.controls.brake = 0.0f;
		ctx.controls.boost = false;
		ctx.controls.shimmyL = false;
		ctx.controls.shimmyR = false;
	}
	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
};

class S_Boosting : public IDriveState {
public:
	void enter(AIDriveContext& ctx) override {
		ctx.controls.reverse = 0.0f;
		ctx.controls.brake = 0.0f;
		ctx.controls.boost = false;
		ctx.controls.shimmyL = false;
		ctx.controls.shimmyR = false;

		ctx.controls.throttle = 1.0f;
		ctx.controls.boost = true;
	}
	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
};

class S_Attacking : public IDriveState {
public:

	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
	std::pair<Direction, glm::vec3> sweepResult{ NONE, glm::vec3(0.f) };
};

class S_Dodging : public IDriveState {
public:

	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
	std::pair<Direction, glm::vec3> sweepResult{ NONE, glm::vec3(0.f) };
};

/*
* Derived state classes.
* These are high-level states used by the ai sparks to determine the 'style' of driving to use.
*/
class DefenseState : public AIState {

public:
	void run(AIDriveContext& ctx) override;
	std::pair<Direction, glm::vec3> detect(AIDriveContext& ctx) override;
	
	std::unique_ptr<IDriveState> currentState = std::make_unique<S_Driving>();
};

class OvertakeState : public AIState {

public:
	void run(AIDriveContext& ctx) override ;
	std::pair<Direction, glm::vec3> detect(AIDriveContext& ctx) override;
};

class MaintainState : public AIState {

public:
	void run(AIDriveContext& ctx) override;
};

