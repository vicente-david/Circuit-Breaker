#pragma once
#include "ai/AISparkComponents.h"
#include "vehicles/SparkComponents.h"
#include "ecs/Component.h"


// For passing as parameters
struct AIDriveContext {
	AIController& ai;
	SparkControls& controls;
	Transform& transform;
	SparkData& spark;
	PxRigidBody* body;
	float healthBoostMin;
};

// Direction enum for labelling sweep/hit direction
enum Direction {
	FWD,
	SIDE_L,
	SIDE_R,
	NONE // no hit
};

/*
* Driving state (lower level of HFSM) interface.
* 
* each state is required to override the update function. This function runs every frame the AI is in that state.
* Optionally, states can override enter function that runs once on changing to that state.
* Sweep result is a pair indicating the direction (enum) and position of a sweep. Not every state uses it.
*/
class IDriveState {
public:
	virtual void enter(AIDriveContext& ctx) {}
	virtual std::unique_ptr<IDriveState> update(AIDriveContext& ctx) = 0;

	std::pair<Direction, glm::vec3> sweepResult;

};
