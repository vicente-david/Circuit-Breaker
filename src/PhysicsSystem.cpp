#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem() // Constructor
{
	// Initialize PhysX
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	if (!gFoundation)
	{
		std::cout << "PxCreateFoundation failed!" << std::endl;
	}

	// PVD
	gPvd = PxCreatePvd(*gFoundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	// Physics
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), true, gPvd);
	if (!gPhysics)
	{
		std::cout << "PxCreatePhysics failed!" << std::endl;
	}

	// Scene
	physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	gGravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.gravity = gGravity;
	gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	// Prep PVD
	physx::PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	// Simulate
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	physx::PxRigidStatic* groundPlane = physx::PxCreatePlane(*gPhysics, physx::PxPlane(0, 1, 0, 50), *gMaterial);
	gScene->addActor(*groundPlane);

	// Define a box
	float halfLen = 0.5f;
	physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);
	physx::PxU32 size = 30;
	physx::PxTransform tran(physx::PxVec3(0));

	// Create a pyramid of physics-enabled boxes
	for (physx::PxU32 i = 0; i < size; i++)
	{
		for (physx::PxU32 j = 0; j < size - i; j++)
		{
			physx::PxTransform localTran(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 - 1), 0) * halfLen);
			physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(tran.transform(localTran));

			rigidDynamicList.push_back(body);
			transformList.push_back(new Transform);

			body->attachShape(*shape);
			physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}

	// Clean up
	shape->release();
}

void PhysicsSystem::updateTransforms()
{
	for (int i = 0; i < transformList.size(); i++)
	{
		// Store positions
		transformList[i]->pos.x = getPos(i).x;
		transformList[i]->pos.y = getPos(i).y;
		transformList[i]->pos.z = getPos(i).z;

		// Store positions
		transformList[i]->rot.x = getRot(i).x;
		transformList[i]->rot.y = getRot(i).y;
		transformList[i]->rot.z = getRot(i).z;
		transformList[i]->rot.w = getRot(i).w;
	}
}

void PhysicsSystem::updatePhysics(double dt) {
	gScene->simulate(dt);
	gScene->fetchResults(true);

	updateTransforms();
}

physx::PxVec3 PhysicsSystem::getPos(int i) const { return rigidDynamicList[i]->getGlobalPose().p; }
physx::PxQuat PhysicsSystem::getRot(int i) const { return rigidDynamicList[i]->getGlobalPose().q; }