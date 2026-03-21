#pragma once
#include "ai/AISparkComponents.h"
#include "vehicles/SparkComponents.h"

// For passing as parameters
struct AIDriveContext {
	AIController& ai;
	SparkControls& controls;
	Transform& transform;
	SparkData& spark;
	PxRigidBody* body;
};

// Driving state interface
class IDriveState {
public:
	virtual void enter(AIDriveContext& ctx) {}
	virtual std::unique_ptr<IDriveState> update(AIDriveContext& ctx) = 0;
};
