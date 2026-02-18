#pragma once

#include "PxPhysicsAPI.h"
#include "Entity.h"
#include "Callbacks.h"
#include "Mesh.h"
#include <vector>
#include <iostream>

#include <ctype.h>

#include "../snippets/snippetvehiclecommon/SnippetVehicleHelpers.h"
#include "../snippets/snippetcommon/SnippetPVD.h"

#include "ecs/Coordinator.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

extern Coordinator coordinator;

class PhysicsSystem : public System
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

	PxTriangleMesh* cookTriangleMesh(Mesh mesh);
	void initStaticMesh(Mesh mesh, Transform transform);

	void updateTransforms(std::vector<Entity> entityList);

	void updatePhysics(double dt, std::vector<Entity> entityList);

	PxVec3 getPos(int i) const; // Get position of id
	PxQuat getRot(int i) const; // Get rotation of id
};
