#pragma once
#include "../snippets/snippetvehiclecommon/enginedrivetrain/EngineDrivetrain.h"
#include "../snippets/snippetvehiclecommon/serialization/BaseSerialization.h"
#include "../snippets/snippetvehiclecommon/serialization/EngineDrivetrainSerialization.h"
#include <memory>

using namespace snippetvehicle;

struct HumanController {
	int controllerNum=0;
};

struct SparkControls {
	float steering; // range [-1,1] for turning left/right
	float throttle; // [0-1] for going forewards
	float brake;
	float reverse; // [0-1] for going backwards

	bool boost;
	bool shimmyL;
	bool shimmyR;

	bool reset;
};

struct SparkData {
	float currBoost = 100;
	float boostRegenSpeed = 10.0f; 

	double ShimmyCooldown = 1;
	double shimmyTimer = 0;

	// this is stuff for physx magic
	std::shared_ptr<EngineDriveVehicle> mVehicle;
	PxVehiclePhysXSimulationContext mVehicleSimContext;

	PxVehiclePhysXMaterialFriction mMaterialFrictions[16];
	PxU32 mNbMaterialFrictions = 0;
	PxReal mDefaultMaterialFriction = 1.0f;

	const char *mVehicleDataPath = NULL;
	const char *mVehicleName = "unnamed_vehicle";

	void destroy() {
		// mVehicle.destroy();
	}
};
