
#include "vehicles/SparkSys.h"
#include "GameState.h"
#include "PxForceMode.h"
#include "PxRigidDynamic.h"
#include "SparkComponents.h"
#include "debugUtils/Logger.h"
#include "ecs/Component.h"
#include "ecs/EntityManager.h"
#include "graphics/Model.h"
#include <cstdio>

void SparkSys::updateSparks(double dt, GameState &game) {
	for (auto const &entity : entities) {
		auto &rBody = game.coordinator->getComponent<PxRigidBody *>(entity);
		auto &sData = game.coordinator->getComponent<SparkData>(entity);
		auto &controls = game.coordinator->getComponent<SparkControls>(entity);

		const PxVec3 linVel = rBody->getLinearVelocity();
		const PxVec3 forwardDir = rBody->getGlobalPose().q.getBasisVector2();

		const PxReal speed = linVel.dot(forwardDir);
		const PxU8 nbSubsteps = (speed < 5.0f ? 3 : 1);

		sData.mVehicle.mCommandState.brakes[0] = controls.brake;
		sData.mVehicle.mCommandState.nbBrakes = 1;
		sData.mVehicle.mCommandState.throttle = controls.throttle;
		sData.mVehicle.mCommandState.steer = controls.steering;

		dbug::log("GAME", -1, "Spark commands: th: %f, brk: %f, trn: %f",
				  controls.throttle, controls.brake, controls.steering);

		sData.mVehicle.mComponentSequence.setSubsteps(
			sData.mVehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);

		sData.mVehicle.step(dt, sData.mVehicleSimContext);
		// rBody->addForce(forwardDir *controls.throttle, PxForceMode::eACCELERATION);
	}
}

EcsEntity SparkSys::createSpark(GameState &game) {

	EcsEntity sparkEntity = game.coordinator->createEntity();

	// if you create a sparkdata object in this function it gets freed, so
	// we need to get a referenct from the ECS coordinator instead (this
	// defintely didn't take hours to debug. I love c)
	game.coordinator->addComponent(sparkEntity, SparkData());
	SparkData &sData = game.coordinator->getComponent<SparkData>(sparkEntity);

	// SparkData sData;
	// Load the params from json or set directly.
	sData.mVehicleDataPath = "assets/vehicledata";
	readBaseParamsFromJsonFile(sData.mVehicleDataPath, "Base.json",
							   sData.mVehicle.mBaseParams);
	readEngineDrivetrainParamsFromJsonFile(sData.mVehicleDataPath,
										   "EngineDrive.json",
										   sData.mVehicle.mEngineDriveParams);
	setPhysXIntegrationParams(
		sData.mVehicle.mBaseParams.axleDescription, sData.mMaterialFrictions,
		sData.mNbMaterialFrictions, sData.mDefaultMaterialFriction,
		sData.mVehicle.mPhysXParams);

	// Set the states to default.
	if (!sData.mVehicle.initialize(
			*game.physics->gPhysics, PxCookingParams(PxTolerancesScale()),
			*game.physics->gMaterial,
			EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE)) {
		return -1;
	}

	// Apply a start pose to the physx actor and add it to the physx scene.
	PxTransform startPose(PxVec3(5.000000000f, -0.000000000f, -40.0f),
						  PxQuat(PxIdentity));
	sData.mVehicle.setUpActor(*game.physics->gScene, startPose,
							  sData.mVehicleName);
	// Create vehicle filter
	PxFilterData vehicleFilter(COLLISION_FLAG_CHASSIS,
							   COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);

	auto rBody = sData.mVehicle.mPhysXState.physxActor.rigidBody;

	// Set flags
	PxU32 shapes = rBody->getNbShapes();
	for (PxU32 i = 0; i < shapes; i++) {
		PxShape *shape = NULL;
		rBody->getShapes(&shape, 1, i);

		shape->setSimulationFilterData(
			vehicleFilter); // Add filter data to shader

		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}

	// Set the vehicle in 1st gear.
	sData.mVehicle.mEngineDriveState.gearboxState.currentGear =
		sData.mVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	sData.mVehicle.mEngineDriveState.gearboxState.targetGear =
		sData.mVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	// Set the vehicle to use the automatic gearbox.
	sData.mVehicle.mTransmissionCommandState.targetGear =
		PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

	// Set up the simulation context.
	// The snippet is set up with
	// a) z as the longitudinal axis
	// b) x as the lateral axis
	// c) y as the vertical axis.
	// d) metres  as the lengthscale.
	sData.mVehicleSimContext.setToDefault();

	sData.mVehicleSimContext.frame.lngAxis = PxVehicleAxes::ePosZ;
	sData.mVehicleSimContext.frame.latAxis = PxVehicleAxes::ePosX;
	sData.mVehicleSimContext.frame.vrtAxis = PxVehicleAxes::ePosY;
	sData.mVehicleSimContext.scale.scale = 1.0f;

	sData.mVehicleSimContext.gravity = game.physics->gGravity;
	sData.mVehicleSimContext.physxScene = game.physics->gScene;

	sData.mVehicleSimContext.physxActorUpdateMode =
		PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;

	SparkControls controls;
	game.coordinator->addComponent(sparkEntity, controls);
	game.coordinator->addComponent(sparkEntity, sData);
	game.coordinator->addComponent(sparkEntity, Transform());
	game.coordinator->addComponent(sparkEntity, rBody);
	game.coordinator->addComponent(sparkEntity, Model("assets/spark.obj"));

	dbug::log("GAME", 0, "Creating a new spark (ID:%d)", sparkEntity);

	return sparkEntity;
}

// helper to register the system
std::shared_ptr<SparkSys>
SparkSys::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<SparkSys>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<SparkControls>());
	sig.set(coord->getComponentType<SparkData>());
	// sig.set(coord->getComponentType<physx::PxRigidDynamic *>());
	coord->setSystemSignature<SparkSys>(sig);

	return system;
}
