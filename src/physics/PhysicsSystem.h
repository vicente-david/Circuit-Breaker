#pragma once

#include "GameState.h"
#include <memory>
#include <vector>

#include "../snippets/snippetvehiclecommon/SnippetVehicleHelpers.h"
#include "PxPhysics.h"
#include "ecs/Coordinator.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

class PhysicsSystem : public System {
  public:
	// PhysX management class instances.
	PxDefaultAllocator gAllocator;
	PxDefaultErrorCallback gErrorCallback;
	PxFoundation *gFoundation = NULL;
	std::shared_ptr<PxPhysics> gPhysics = NULL;
	PxDefaultCpuDispatcher *gDispatcher = NULL;
	PxScene *gScene = NULL;
	PxMaterial *gMaterial = NULL;
	PxPvd *gPvd = NULL;

	// The mapping between PxMaterial and friction.
	PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
	PxU32 gNbPhysXMaterialFrictions = 0;
	PxReal gPhysXDefaultMaterialFriction = 1.0f;

	PxRigidStatic *gGroundPlane = NULL;
	const PxVec3 gGravity = PxVec3(0.0f, -9.81f, 0.0f);

	PhysicsSystem(); // Constructor

	void createTestObjs(std::shared_ptr<Coordinator> coordinator);

	void initPhysX();
	void initMaterialFrictionTable();

	PxTriangleMesh *cookTriangleMesh(Mesh mesh);
	void initStaticMesh(Mesh mesh, Transform transform);

	void updateTransforms(GameState &state);

	void updatePhysics(double dt, GameState &gameState);

	static std::shared_ptr<PhysicsSystem>
	registerSystem(std::shared_ptr<Coordinator> &coord);
	// this is done through ecs now
	// PxVec3 getPos(int i) const; // Get position of id
	// PxQuat getRot(int i) const; // Get rotation of id
	//
};
