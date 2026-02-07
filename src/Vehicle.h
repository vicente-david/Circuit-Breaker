//#pragma once
//#include "PxPhysicsAPI.h"
//#include "PhysicsSystem.h"
//
//#include "../snippets/snippetvehiclecommon/enginedrivetrain/EngineDrivetrain.h"
//#include "../snippets/snippetvehiclecommon/serialization/BaseSerialization.h"
//#include "../snippets/snippetvehiclecommon/serialization/EngineDrivetrainSerialization.h"
//
//#include "../snippets/snippetcommon/SnippetPVD.h"
//
//struct Command
//{
//	physx::PxF32 brake;
//	physx::PxF32 throttle;
//	physx::PxF32 steer;
//	physx::PxU32 gear;
//};
//
//class Vehicle
//{
//public:
//	//PhysX management class instances.
//	Vehicle(PhysicsSystem& physicsSystem);
//
//	bool init();
//	void cleanup();
//	void step(double dt);
//
//	void setInput(const Command& cmd);
//
//private:
//	PhysicsSystem& mPhysics;
//
//	snippetvehicle::EngineDriveVehicle mVehicle;
//	physx::vehicle2::PxVehiclePhysXSimulationContext mVehicleSimContext;
//
//	physx::vehicle2::PxVehiclePhysXMaterialFriction mMaterialFrictions[16];
//	physx::PxU32 mNbMaterialFrictions = 0;
//	physx::PxReal mDefaultMaterialFriction = 1.0f;
//
//	const char* mVehicleName = "unnamed_vehicle";
//};