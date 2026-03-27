#pragma once
#include "ai/AISparkComponents.h"
#include "vehicles/SparkComponents.h"
#include "ecs/Component.h"
#include "IDriveState.h"


// Helper functions for any state
namespace AIHelpers {
	void calcSteering(AIController& ai, SparkControls& controls, Transform& transform, SparkData& spark, glm::vec3& targetPos);
	std::pair<bool, glm::vec3> lookFwd(Transform& transform, PxRigidBody* body);
	std::pair<bool, glm::vec3> lookSide(Transform& transform, PxRigidBody* body, Direction& dir);
}

/*
* Lower level of HFSM
* All of these states are actions that the AI spark can take.
* The states are derived from the abstract class IDriveState.
* 
*/

class S_Driving : public IDriveState {
public:
	void enter(AIDriveContext& ctx) override {
		// Zero out controls unused in this state
		ctx.controls.reverse = 0.0f;
		ctx.controls.brake = 0.0f;
		ctx.controls.boost = false;
		ctx.controls.boostWithHealth = false;
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
		ctx.controls.boostWithHealth = false;
		ctx.controls.shimmyL = false;
		ctx.controls.shimmyR = false;
	}
	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
};

class S_Drifting : public IDriveState {
public:
	void enter(AIDriveContext& ctx) override {
		ctx.controls.throttle = 1.0f;
		ctx.controls.reverse = 0.0f;
		ctx.controls.brake = 0.0f;
		ctx.controls.boost = false;
		ctx.controls.boostWithHealth = false;
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
		ctx.controls.shimmyL = false;
		ctx.controls.shimmyR = false;

		ctx.controls.throttle = 1.0f;
		
		
	}
	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;


};

class S_Attacking : public IDriveState {
public:
	void enter(AIDriveContext& ctx) override {
		ctx.controls.throttle = 1.0f;
}
	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
	std::pair<Direction, glm::vec3> sweepResult{ NONE, glm::vec3(0.f) };

private:
	const float boostAtkMaxLength = 10.f; // The max amount of time the ai will boost to attack

};

class S_Dodging : public IDriveState {
public:
	void enter(AIDriveContext& ctx) override {
		ctx.controls.throttle = 1.0f;
	}
	std::unique_ptr<IDriveState> update(AIDriveContext& ctx) override;
	std::pair<Direction, glm::vec3> sweepResult{ NONE, glm::vec3(0.f) };

private:
	const float dodgeMaxLength = 10.f; // max amount of time the ai will boost to dodge
};


