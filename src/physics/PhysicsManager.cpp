
#include "PhysicsManager.h"
#include "../snippets/snippetcommon/SnippetPVD.h"
#include "GameState.h"
#include "PxActor.h"
#include "PxPhysics.h"
#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "PxVisualizationParameter.h"
#include "debugUtils/Logger.h"
#include "debugUtils/Panel.h"
#include "ecs/Component.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include "graphics/Model.h"
#include "physics/Callbacks.h"
#include "vehicles/SparkComponents.h"
#include <memory>

// this class acts as an interface between physx and the rest of the game. it
// holds the physx scene, and other global physics things that systems might
// need (for example to create a new physics object)
//
// It may be handy to make some method to just make a rigidbody for you without
// needling like 1000 steps like it is currenlty, but i haven't done that yet
PhysicsManager::PhysicsManager() {

	initPhysX();
	initMaterialFrictionTable();
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

	callbacks = std::make_shared<PhysXCallbacks>();
	// PhysXCallbacks *gContactReportCallback = new PhysXCallbacks();
	sceneDesc.simulationEventCallback =
		callbacks.get(); // Assign callback to scene

	modCallbacks = std::make_shared<ModifiedCallbacks>();
	sceneDesc.contactModifyCallback = modCallbacks.get();

	gScene = gPhysics->createScene(sceneDesc);

	// Prep PVD
	PxPvdSceneClient *pvdClient = gScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES,
								   true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.1f);

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
	gPhysXDefaultMaterialFriction = 40.0f;
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

PxRigidStatic *PhysicsManager::initStaticMesh(Mesh mesh, Transform transform, PxMaterial *material, PxFilterData filter, bool tireCollision) {
	PxTriangleMesh *triangleMesh = cookTriangleMesh(mesh);

	PxMeshScale scale(PxVec3(1, 1, 1), PxQuat(PxIdentity));
	PxTriangleMeshGeometry triGeom(triangleMesh, scale,
								   PxMeshGeometryFlag::eTIGHT_BOUNDS);

	PxShape *triMeshShape = gPhysics->createShape(triGeom, *material);
		triMeshShape->setSimulationFilterData(filter);
	PxRigidStatic *actor = gPhysics->createRigidStatic(PxTransform(PxVec3(0)));
	if(!tireCollision){
		triMeshShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
	}
	actor->attachShape(*triMeshShape);

	// add ground collision filter to all the shapes on the ground mesh
	for (PxU32 i = 0; i < actor->getNbShapes(); i++) {
		PxShape *shape = NULL;
		actor->getShapes(&shape, 1, i);
		printf("i:%d na:%d\n", i, shape->getGeometry().getType());
	}
	gScene->addActor(*actor);
	triMeshShape->release();
	return actor;
}
PxRigidStatic* PhysicsManager::initStaticMesh(Mesh mesh, Transform transform, bool tireCollision) {
	PxFilterData groundFilter(COLLISION_FLAG_GROUND,COLLISION_FLAG_GROUND_AGAINST, 0, 0);
	return initStaticMesh(mesh, transform, gMaterial, groundFilter,tireCollision);
}
PxRigidStatic* PhysicsManager::initHealZones(Mesh mesh, Transform transform) {
	PxFilterData groundHealFilter(COLLISION_FLAG_HEAL, COLLISION_FLAG_WHEEL, 0, 0);
	return initStaticMesh(mesh, transform, gMaterial, groundHealFilter);
}
PxRigidStatic* PhysicsManager::initWalls(Mesh mesh, Transform transform) {
	PxFilterData filter(COLLISION_FLAG_WALL, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
	PxMaterial* wallMat = gPhysics->createMaterial(0.1f, 0.1f, 0.8f); //make walls bouncier

	return initStaticMesh(mesh, transform, wallMat, filter);
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

	physx::PxU32 size = 15;
	physx::PxTransform tran(physx::PxVec3(0));

	Model cube("assets/cube.obj");
	Model spark("assets/spark.obj");
	// Create a pyramid of physics-enabled boxes
	for (physx::PxU32 i = 0; i < size; i++) {
		for (physx::PxU32 j = 0; j < size - i; j++) {
			physx::PxTransform localTran(
				physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i),
							  physx::PxReal(i * 2), 0) *
				halfLen);

			PxRigidBody *body =
				gPhysics->createRigidDynamic(tran.transform(localTran));

			// give different models
			body->attachShape(*shape);
			physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);

			// add to the ecs
			Entity ent = coordinator.createEntity();
			coordinator.addComponent(ent, Transform());
			coordinator.addComponent(ent, body);
			if (j % 2 == 0) {
				coordinator.addComponent(ent, cube);
			} else {
				coordinator.addComponent(ent, spark);
			}

			dbug::log("PHYS", 0, "Box id:%d", ent);

			// add collision info to physics actor
			CollisionData collData{UserPhysicsType::TESTING, ent};
			body->userData = &collData;
		}
	}
	shape->release();
}
void PhysicsManager::updatePhysics(double dt) {
	gScene->simulate(dt);
	gScene->fetchResults(true);

	// enable debug info when dbug panel is activated
	if(!debugEnabled && dbugPanel::enabled){
		gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1);
		gScene->setVisualizationParameter(
			PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	}
}
