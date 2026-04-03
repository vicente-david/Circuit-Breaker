#pragma once

#include "../snippets/snippetvehiclecommon/SnippetVehicleHelpers.h"
#include "PxPhysics.h"
#include "PxRigidStatic.h"
#include "ecs/Component.h"
#include "ecs/Coordinator.h"
#include "graphics/Mesh.h"
#include "physics/Callbacks.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

// this class acts as an interface between physx and the rest of the game, and
// holds the physx scene and everything
class PhysicsManager {
  public:
	// PhysX management class instances.
	PxDefaultAllocator gAllocator;
	PxDefaultErrorCallback gErrorCallback;
	PxFoundation *gFoundation = NULL;
	PxPhysics *gPhysics = NULL;
	PxDefaultCpuDispatcher *gDispatcher = NULL;
	PxScene *gScene = NULL;
	PxMaterial *gMaterial = NULL;
	PxPvd *gPvd = NULL;
	std::shared_ptr<PhysXCallbacks> callbacks;

	// The mapping between PxMaterial and friction.
	PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
	PxU32 gNbPhysXMaterialFrictions = 0;
	PxReal gPhysXDefaultMaterialFriction = 1.0f;

	PxRigidStatic *gGroundPlane = NULL;
	const PxVec3 gGravity = PxVec3(0.0f, -9.81f, 0.0f);

	PhysicsManager(); // Constructor

	void initPhysX();
	void initMaterialFrictionTable();

	PxTriangleMesh *cookTriangleMesh(Mesh mesh);
	PxRigidStatic* initStaticMesh(Mesh mesh, Transform transform, PxMaterial *material, PxFilterData fitler, bool tireCollision = true);
	PxRigidStatic* initStaticMesh(Mesh mesh, Transform transform, bool tireCollision = true);
	PxRigidStatic* initHealZones(Mesh mesh, Transform transform);

	void updatePhysics(double dt);

	void createTestObjs(Coordinator &coordinator);
};
