#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem() // Constructor
{

	initPhysX();
	initGroundPlane();
	initMaterialFrictionTable();

	// Define a box
	float halfLen = 0.5f;
	physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);

	PxFilterData boxFilter(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0); // Create obstacle filter
	shape->setSimulationFilterData(boxFilter); // Add filter data to shader

	physx::PxU32 size = 30;
	physx::PxTransform tran(physx::PxVec3(0));

	// Create a pyramid of physics-enabled boxes
	transformList.reserve(465);
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

void PhysicsSystem::initPhysX()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	// Scene
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = gGravity;

	PxU32 numWorkers = 1;
	gDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = VehicleFilterShader;

	ContactReportCallback* gContactReportCallback = new ContactReportCallback();
	sceneDesc.simulationEventCallback = gContactReportCallback; // Assign callback to scene

	gScene = gPhysics->createScene(sceneDesc);

	// Prep PVD
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxInitVehicleExtension(*gFoundation); // Initialize vehicle extension
}

void PhysicsSystem::initGroundPlane()
{
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);

	for (PxU32 i = 0; i < groundPlane->getNbShapes(); i++) {
		PxShape* shape = nullptr;
		groundPlane->getShapes(&shape, 1, i);
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}
	gScene->addActor(*groundPlane);
}

void PhysicsSystem::initMaterialFrictionTable()
{
	//Each physx material can be mapped to a tire friction value on a per tire basis.
	//If a material is encountered that is not mapped to a friction value, the friction value used is the specified default value.
	//In this snippet there is only a single material so there can only be a single mapping between material and friction.
	//In this snippet the same mapping is used by all tires.
	gPhysXMaterialFrictions[0].friction = 1.0f;
	gPhysXMaterialFrictions[0].material = gMaterial;
	gPhysXDefaultMaterialFriction = 1.0f;
	gNbPhysXMaterialFrictions = 1;
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