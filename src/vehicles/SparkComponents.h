#pragma once
#include "../snippets/snippetvehiclecommon/enginedrivetrain/EngineDrivetrain.h"
#include "../snippets/snippetvehiclecommon/serialization/BaseSerialization.h"
#include "../snippets/snippetvehiclecommon/serialization/EngineDrivetrainSerialization.h"
#include "audio/AudioEngine.h"
#include "audio/Sound.h"
#include "physics/CollisionData.h"
#include "world/CurveLoader.h"

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

	bool driftMode;
	bool boost;
	bool boostWithHealth;
	bool shimmyL;
	bool shimmyR;

	bool reset;
	bool reload;
};

// this is basically the current state of the spark. things can sometimes break
// if you forget to use refences to this stuff because physx makes no sense
struct SparkData {
	float maxHealth = 100.0f;
	float health = maxHealth;
	float healthRegenRate = 10.f;

	float boostUseRate = 20.f;
	float boostStrength = 10.0f;
	float boostRegenRate = 20.0f;
	float maxBoost = 100.0f;
	float boost = maxBoost;

	float shimmyForce = 15.0f;
	double shimmyCooldown = 2;
	double shimmyInvincible = shimmyCooldown - 0.5;
	double shimmyTimer = 0;

	float speed = 0.0f;
	float minDriftSpeed = 10.f;

	double respawnCooldown = 3;
	double respawnTimer = 0;
	double deathCooldown = 2;
	double deathTimer = deathCooldown;
	double offGroundLimit = 2;
	double offGroundTimer = offGroundLimit;
	double angResTimer = 0;

	bool inReverse = false;
	bool inDrift = false;
	bool isBoosting = false;
	bool isDead = false;
	bool isGrounded = false;
	bool isOffroad = false;

	PathID path = pFAST;

	// this is stuff for physx magic
	std::shared_ptr<EngineDriveVehicle> mVehicle;
	PxVehiclePhysXSimulationContext mVehicleSimContext;

	// Easy access to vehicle's neutral gear
	PxU32 neutralGear = 0;

	// Easy access to vehicle's rigid body
	PxRigidBody *rBody = NULL;

	PxVehiclePhysXMaterialFriction mMaterialFrictions[16];
	PxU32 mNbMaterialFrictions = 0;
	PxReal mDefaultMaterialFriction = 1.0f;

	// sweeps for wheels instead of ray cast (cylinder with unit size)
	PxConvexMesh *mCylinderSweepMesh = NULL;

	const char *mVehicleDataPath = NULL;
	std::string mVehicleName = "unnamed_vehicle";

	CollisionData physData = CollisionData{SPARK, -1};
	void destroy() {
		if (mCylinderSweepMesh)
			PxVehicleUnitCylinderSweepMeshDestroy(mCylinderSweepMesh);

		mVehicle->destroy();
	}

	bool isHuman = false;
};
struct SparkSounds {
	std::shared_ptr<Sound> engine;
	float currBoostPitch = 0;
	constexpr static float maxBoostPitch = 1;
	constexpr static float boostPitchSpeed = 3;

	SparkSounds() {};
	SparkSounds(std::shared_ptr<AudioEngine> audio) {
		engine = audio->createSound("engine");
	};
};
