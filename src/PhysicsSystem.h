#pragma once

#include "PxPhysicsAPI.h"
#include "Entity.h"
#include "Callbacks.h"
#include <vector>
#include <iostream>

#include <ctype.h>

#include "../snippets/snippetvehiclecommon/SnippetVehicleHelpers.h"
#include "../snippets/snippetcommon/SnippetPVD.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

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

	//The mapping between PxMaterial and friction.
	PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
	PxU32 gNbPhysXMaterialFrictions = 0;
	PxReal gPhysXDefaultMaterialFriction = 1.0f;

	PxRigidStatic* gGroundPlane = NULL;
	const PxVec3 gGravity = PxVec3(0.0f, -9.81f, 0.0f);

	std::vector<physx::PxRigidDynamic*> rigidDynamicList;
	std::vector<Transform*> transformList;

	PhysicsSystem(); // Constructor

	void initPhysX();
	void initGroundPlane();
	void initMaterialFrictionTable();

	void updateTransforms();

	void updatePhysics(double dt);

	PxVec3 getPos(int i) const; // Get position of id
	PxQuat getRot(int i) const; // Get rotation of id
};
