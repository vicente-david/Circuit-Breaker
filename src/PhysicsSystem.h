#pragma once

#include "PxPhysicsAPI.h"
#include "Entity.h"
#include <vector>
#include <iostream>

#include <ctype.h>

#include "../snippets/snippetvehiclecommon/enginedrivetrain/EngineDrivetrain.h"
#include "../snippets/snippetvehiclecommon/serialization/BaseSerialization.h"
#include "../snippets/snippetvehiclecommon/serialization/EngineDrivetrainSerialization.h"
#include "../snippets/snippetvehiclecommon/SnippetVehicleHelpers.h"

#include "../snippets/snippetcommon/SnippetPVD.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

//Commands are issued to the vehicle in a pre-choreographed sequence.
struct Command
{
	PxF32 brake;
	PxF32 throttle;
	PxF32 steer;
	PxU32 gear;
	PxF32 duration;
};

class PhysicsSystem
{
public:
	//PhysX management class instances.

	PxDefaultAllocator		gAllocator;
	PxDefaultErrorCallback	gErrorCallback;
	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;
	PxDefaultCpuDispatcher* gDispatcher = NULL;
	PxScene* gScene = NULL;
	PxMaterial* gMaterial = NULL;
	PxPvd* gPvd = NULL;

	//The path to the vehicle json files to be loaded.
	const char* gVehicleDataPath = NULL;

	//The vehicle with engine drivetrain
	EngineDriveVehicle gVehicle;

	//Vehicle simulation needs a simulation context
	//to store global parameters of the simulation such as 
	//gravitational acceleration.
	PxVehiclePhysXSimulationContext gVehicleSimulationContext;

	//Gravitational acceleration
	const physx::PxVec3 gGravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

	//The mapping between PxMaterial and friction.
	PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
	PxU32 gNbPhysXMaterialFrictions = 0;
	PxReal gPhysXDefaultMaterialFriction = 1.0f;

	//Give the vehicle a name so it can be identified in PVD.
	const char gVehicleName[12] = "engineDrive";


	const PxU32 gTargetGearCommand = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;
	Command gCommands[5] =
	{
		{0.5f, 0.0f, 0.0f, gTargetGearCommand, 2.0f},	//brake on and come to rest for 2 seconds
		{0.0f, 0.65f, 0.0f, gTargetGearCommand, 5.0f},	//throttle for 5 seconds
		{0.5f, 0.0f, 0.0f, gTargetGearCommand, 5.0f},	//brake for 5 seconds
		{0.0f, 0.75f, 0.0f, gTargetGearCommand, 5.0f},	//throttle for 5 seconds
		{0.0f, 0.25f, 0.5f, gTargetGearCommand, 5.0f}	//light throttle and steer for 5 seconds.
	};
	const PxU32 gNbCommands = sizeof(gCommands) / sizeof(Command);
	PxReal gCommandTime = 0.0f;			//Time spent on current command
	PxU32 gCommandProgress = 0;			//The id of the current command.

	//A ground plane to drive on.
	PxRigidStatic* gGroundPlane = NULL;

	std::vector<physx::PxRigidDynamic*> rigidDynamicList;
	std::vector<Transform*> transformList;

	////PhysX management class instances.
	//physx::PxDefaultAllocator gAllocator;
	//physx::PxDefaultErrorCallback gErrorCallback;
	//physx::PxFoundation* gFoundation = NULL;
	//physx::PxPhysics* gPhysics = NULL;
	//physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
	//physx::PxScene* gScene = NULL;
	//physx::PxMaterial* gMaterial = NULL;
	//physx::PxPvd* gPvd = NULL;

	PhysicsSystem(); // Constructor

	void initPhysX();
	void initGroundPlane();
	void initMaterialFrictionTable();
	bool initVehicles();
	void stepPhysics(PxReal timestep);

	void updateTransforms();

	void updatePhysics(double dt);

	physx::PxVec3 getPos(int i) const; // Get position of id
	physx::PxQuat getRot(int i) const; // Get rotation of id
};
