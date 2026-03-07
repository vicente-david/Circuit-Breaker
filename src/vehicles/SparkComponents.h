#pragma once
#include "../snippets/snippetvehiclecommon/enginedrivetrain/EngineDrivetrain.h"
#include "../snippets/snippetvehiclecommon/serialization/BaseSerialization.h"
#include "../snippets/snippetvehiclecommon/serialization/EngineDrivetrainSerialization.h"
#include <memory>
#include "physics/CollisionData.h"

using namespace snippetvehicle;

// this component tells the controller system that this car is piloted by a
// controller, not an AI. I also added the controller number for if we can to
// multiplayer though
struct HumanController {
	int controllerNum = 0;
};

// this stores the controls for piloting a spark. they can be changed wither by
// an AI or controller input. this could propably be compined with sparkData,
// but they feel better seperate to me
struct SparkControls {
	float steering; // range [-1,1] for turning left/right
	float throttle; // [0-1] for going forewards
	float brake;
	float reverse; // [0-1] for going backwards

	bool handbrake;
	bool boost;
	bool shimmyL;
	bool shimmyR;

	bool reset;
	bool reload;
};

// this is basically the current state of the spark. things can sometimes break
// if you forget to use refences to this stuff because physx makes no sense
struct SparkData {
	float health = 100;
	float currBoost = 100;
	float boostRegenSpeed = 10.0f;

	double ShimmyCooldown = 1;
	double shimmyTimer = 0;

	float speed = 0.0f;

	// this is stuff for physx magic
	std::shared_ptr<EngineDriveVehicle> mVehicle;
	PxVehiclePhysXSimulationContext mVehicleSimContext;

	PxVehiclePhysXMaterialFriction mMaterialFrictions[16];
	PxU32 mNbMaterialFrictions = 0;
	PxReal mDefaultMaterialFriction = 1.0f;

	const char *mVehicleDataPath = NULL;
	const char *mVehicleName = "unnamed_vehicle";

	CollisionData physData = CollisionData{SPARK, -1};
	void destroy() {
		mVehicle->destroy();
	}
};
