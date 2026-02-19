
#include "PhysicsManager.h"
#include "../snippets/snippetcommon/SnippetPVD.h"
#include "Callbacks.h"
#include "GameState.h"
#include "PxPhysics.h"
#include "PxRigidDynamic.h"
#include "Transform.h"
#include "debugUtils/Logger.h"
#include "ecs/Component.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include <memory>

PhysicsManager::PhysicsManager() // Constructor
{

	dbug::log("PHYS", 0, "initializing physics");
	initPhysX();
	// initGroundPlane();
	initMaterialFrictionTable();

	// Define a box
	float halfLen = 0.5f;
	physx::PxShape *shape = gPhysics->createShape(
		physx::PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);

	PxFilterData boxFilter(COLLISION_FLAG_OBSTACLE,
						   COLLISION_FLAG_OBSTACLE_AGAINST, 0,
						   0);				   // Create obstacle filter
	shape->setSimulationFilterData(boxFilter); // Add filter data to shader

	physx::PxU32 size = 30;
	physx::PxTransform tran(physx::PxVec3(0));

	// // Create a pyramid of physics-enabled boxes
	// transformList.reserve(465);
	// for (physx::PxU32 i = 0; i < size; i++) {
	// 	for (physx::PxU32 j = 0; j < size - i; j++) {
	// 		physx::PxTransform localTran(
	// 			physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i),
	// 						  physx::PxReal(i * 2 - 1), 0) *
	// 			halfLen);
	// 		physx::PxRigidDynamic *body =
	// 			gPhysics->createRigidDynamic(tran.transform(localTran));
	//
	// 		rigidDynamicList.push_back(body);
	// 		transformList.push_back(new Transform);
	//
	// 		body->attachShape(*shape);
	// 		physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	// 		gScene->addActor(*body);
	// 	}
	// }
	// create 'finish line' trigger box
	PxVec3 finishLinePosition(0.0f, 0.0f,
							  10.0f); // finish line position in world space
	PxVec3 triggerLengths(255.637f, 100.0f,
						  1.0f); // width, height, and depth of the finish line
	PxRigidStatic *triggerActor = gPhysics->createRigidStatic(PxTransform(
		finishLinePosition)); // create static rigid body for the trigger box

	physx::PxShape *triggerRect = gPhysics->createShape(
		physx::PxBoxGeometry(triggerLengths), *gMaterial, true);

	// set the shape as a trigger
	triggerRect->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	triggerRect->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

	triggerActor->attachShape(*triggerRect);
	gScene->addActor(*triggerActor);

	PxFilterData finishLineTriggerFilterData;
	finishLineTriggerFilterData.word0 =
		99; // it detects only the player vehicle
	triggerRect->setSimulationFilterData(finishLineTriggerFilterData);

	// Clean up
	shape->release();

	triggerRect->release();
}

void PhysicsManager::initPhysX() {
	dbug::log("PHYS", 0, "Initializing Physx");
	gFoundation =
		PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport *transport =
		PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation,
							   PxTolerancesScale(), true, gPvd);

	// Scene
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = gGravity;

	PxU32 numWorkers = 1;
	gDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = VehicleFilterShader;

	ContactReportCallback *gContactReportCallback = new ContactReportCallback();
	sceneDesc.simulationEventCallback =
		gContactReportCallback; // Assign callback to scene

	gScene = gPhysics->createScene(sceneDesc);

	// Prep PVD
	PxPvdSceneClient *pvdClient = gScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES,
								   true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxInitVehicleExtension(*gFoundation); // Initialize vehicle extension
}

void PhysicsManager::initMaterialFrictionTable() {
	// Each physx material can be mapped to a tire friction value on a per tire
	// basis. If a material is encountered that is not mapped to a friction
	// value, the friction value used is the specified default value. In this
	// snippet there is only a single material so there can only be a single
	// mapping between material and friction. In this snippet the same mapping
	// is used by all tires.
	gPhysXMaterialFrictions[0].friction = 1.0f;
	gPhysXMaterialFrictions[0].material = gMaterial;
	gPhysXDefaultMaterialFriction = 1.0f;
	gNbPhysXMaterialFrictions = 1;
}

PxTriangleMesh *PhysicsManager::cookTriangleMesh(Mesh mesh) {
	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = mesh.vertices.size();
	meshDesc.points.stride = sizeof(Vertex);
	meshDesc.points.data = mesh.vertices.data();

	meshDesc.triangles.count = mesh.indices.size() / 3;
	meshDesc.triangles.stride = 3 * sizeof(PxU32);
	meshDesc.triangles.data = mesh.indices.data();

	PxTolerancesScale scale;
	PxCookingParams params(scale);

	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
	if (!status)
		return NULL;

	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(),
										writeBuffer.getSize());
	return gPhysics->createTriangleMesh(readBuffer);
}

void PhysicsManager::initStaticMesh(Mesh mesh, Transform transform) {
	PxTriangleMesh *triangleMesh = cookTriangleMesh(mesh);

	PxMeshScale scale(PxVec3(1, 1, 1), PxQuat(PxIdentity));
	PxTriangleMeshGeometry triGeom(triangleMesh, scale,
								   PxMeshGeometryFlag::eTIGHT_BOUNDS);

	PxShape *triMeshShape = gPhysics->createShape(triGeom, *gMaterial);
	PxRigidStatic *actor = gPhysics->createRigidStatic(PxTransform(PxVec3(0)));
	actor->attachShape(*triMeshShape);

	gScene->addActor(*actor);
	triMeshShape->release();
}

void PhysicsManager::createTestObjs(Coordinator &coordinator) {
	dbug::log("PHYS", 0, "Creating test objects");
	float halfLen = 0.5f;
	physx::PxShape *shape = gPhysics->createShape(
		physx::PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);

	PxFilterData boxFilter(COLLISION_FLAG_OBSTACLE,
						   COLLISION_FLAG_OBSTACLE_AGAINST, 0,
						   0);				   // Create obstacle filter
	shape->setSimulationFilterData(boxFilter); // Add filter data to shader

	physx::PxU32 size = 4;
	physx::PxTransform tran(physx::PxVec3(0));

	Model cube("assets/cube.obj");
	Model spark("assets/spark.obj");
	// Create a pyramid of physics-enabled boxes
	for (physx::PxU32 i = 0; i < size; i++) {
		for (physx::PxU32 j = 0; j < size - i; j++) {
			physx::PxTransform localTran(
				physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i),
							  physx::PxReal(i * 2 ), 0) *
				halfLen);
			PxRigidDynamic *b;
			b = gPhysics->createRigidDynamic(tran.transform(localTran));

			// give different models
			b->attachShape(*shape);
			physx::PxRigidBodyExt::updateMassAndInertia(*b, 10.0f);
			gScene->addActor(*b);

			// add to the ecs
			EcsEntity ent = coordinator.createEntity();
			coordinator.addComponent(ent, Transform());
			coordinator.addComponent(ent, b);
			if (j % 2 == 0) {
				coordinator.addComponent(ent, cube);
			} else {
				coordinator.addComponent(ent, spark);
			}
			dbug::log("PHYS", 0, "Box id:%d", ent);
		}
	}
	shape->release();
}
void PhysicsManager::updatePhysics(double dt) {
	gScene->simulate(dt);
	gScene->fetchResults(true);
}
