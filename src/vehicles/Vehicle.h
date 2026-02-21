#pragma once
#include "GameState.h"
#include "InputSystem.h"
#include "PxPhysicsAPI.h"
#include "physics/PhysicsSystem.h"
#include <memory>
#include <vector>
#include <iostream>

#include "../snippets/snippetvehiclecommon/enginedrivetrain/EngineDrivetrain.h"
#include "../snippets/snippetvehiclecommon/serialization/BaseSerialization.h"
#include "../snippets/snippetvehiclecommon/serialization/EngineDrivetrainSerialization.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

//struct Command
//{
//	PxF32 brake;
//	PxF32 throttle;
//	PxF32 steer;
//	PxU32 gear;
//	bool boost;
//	// add more when needed
//};


// this shouldn't be needed anymore, just keeping it for reference incase things break
class Vehicle
{
public:
	//PhysX management class instances.
	Vehicle(std::shared_ptr<PhysicsManager> physicsSystem);

	bool init();
	void cleanup();
	void step(double dt);

	void applyInput(Actions& actions, double currentTime);
	void changeEngineDriveParams(const char* vehicleDataPath);
	void Boost();
	void Shimmy(bool rightDir);
	void respawn();

	std::shared_ptr<PhysicsManager> mPhysics;

	EngineDriveVehicle mVehicle;
	PxVehiclePhysXSimulationContext mVehicleSimContext;

	PxVehiclePhysXMaterialFriction mMaterialFrictions[16];
	PxU32 mNbMaterialFrictions = 0;
	PxReal mDefaultMaterialFriction = 1.0f;

	const char* mVehicleDataPath = NULL;
	const char* mVehicleName = "unnamed_vehicle";
	Transform transform;

private:
	void updateTransform();

	PxRigidBody* rBody; // Vehicle actor rigid body

	double shimmyCooldown = 1; // Time before you can shimmy again
	double shimmyActiveTimer = 0; // Time left before you can shimmy

	float boostMax = 100.f; // Maximum amount of boost
	float boostAmount = boostMax; // Current amount of boost
	float boostRegenerate = 0.3f; // Regeneration rate
};
