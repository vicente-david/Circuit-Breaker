#include "PxPhysicsAPI.h"
#include "Entity.h"
#include <vector>
#include <iostream>

class PhysicsSystem 
{
public:

	std::vector<physx::PxRigidDynamic*> rigidDynamicList;
	std::vector<Transform*> transformList;

	//PhysX management class instances.
	physx::PxDefaultAllocator gAllocator;
	physx::PxDefaultErrorCallback gErrorCallback;
	physx::PxFoundation* gFoundation = NULL;
	physx::PxPhysics* gPhysics = NULL;
	physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
	physx::PxScene* gScene = NULL;
	physx::PxMaterial* gMaterial = NULL;
	physx::PxPvd* gPvd = NULL;

	PhysicsSystem(); // Constructor

	void updateTransforms(std::vector<Entity> entityList);

	void updatePhysics(double dt, std::vector<Entity> entityList);

	physx::PxVec3 getPos(int i); // Get position of id
	physx::PxQuat getRot(int i); // Get rotation of id
};
