
#include "vehicles/SparkSys.h"
#include "GameState.h"
#include "PxRigidDynamic.h"
#include "SparkComponents.h"
#include "debugUtils/Logger.h"
#include "ecs/Component.h"
#include "ecs/EntityManager.h"
#include "graphics/Model.h"
#include <cstdio>

void SparkSys::updateSparks(double dt, GameState &game) {
	for (auto const &entity : entities) {
		auto& rBody = game.coordinator->getComponent<PxRigidBody *>(entity);
		auto& sparkData = game.coordinator->getComponent<SparkData>(entity);
		auto& controls = game.coordinator->getComponent<SparkControls>(entity);

		const PxVec3 linVel = rBody->getLinearVelocity();
		const PxVec3 forwardDir = rBody->getGlobalPose().q.getBasisVector2();

		const PxReal speed = linVel.dot(forwardDir);
		const PxU8 nbSubsteps = (speed < 5.0f ? 3 : 1);

		// printf("spark phys: %p\n", sparkData.mVehicleSimContext.physxUnitCylinderSweepMesh);
		// if( sparkData.mVehicleSimContext.physxUnitCylinderSweepMesh == NULL){
			printf("OH NOE!\n");
		// }
		sparkData.mVehicle.mComponentSequence.setSubsteps(
			sparkData.mVehicle.mComponentSequenceSubstepGroupHandle,
			nbSubsteps);

		sparkData.mVehicle.step(dt, sparkData.mVehicleSimContext);
	}
}

EcsEntity SparkSys::createSpark(GameState &game) {

	SparkData sData;
	//Load the params from json or set directly.
	sData.mVehicleDataPath = "assets/vehicledata";
	readBaseParamsFromJsonFile(sData.mVehicleDataPath, "Base.json", sData.mVehicle.mBaseParams);
	readEngineDrivetrainParamsFromJsonFile(sData.mVehicleDataPath, "EngineDrive.json", sData.mVehicle.mEngineDriveParams);
	setPhysXIntegrationParams(
		sData.mVehicle.mBaseParams.axleDescription,
		sData.mMaterialFrictions,
		sData.mNbMaterialFrictions,
		sData.mDefaultMaterialFriction,
		sData.mVehicle.mPhysXParams
	);

	//Set the states to default.
	if (!sData.mVehicle.initialize(
		*game.physics->gPhysics,
		PxCookingParams(PxTolerancesScale()),
		*game.physics->gMaterial,
		EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE
		))
	{
		return -1;
	}

	//Apply a start pose to the physx actor and add it to the physx scene.
	PxTransform startPose(PxVec3(0.000000000f, -0.000000000f, -50.59399998f), PxQuat(PxIdentity));
	sData.mVehicle.setUpActor(*game.physics->gScene, startPose, sData.mVehicleName);
	// Create vehicle filter
	PxFilterData vehicleFilter(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
	
	auto rBody = sData.mVehicle.mPhysXState.physxActor.rigidBody;

	// Set flags
	PxU32 shapes = rBody->getNbShapes();
	for (PxU32 i = 0; i < shapes; i++) {
		PxShape* shape = NULL;
		rBody->getShapes(&shape, 1, i);

		shape->setSimulationFilterData(vehicleFilter); // Add filter data to shader

		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}

	//Set the vehicle in 1st gear.
	sData.mVehicle.mEngineDriveState.gearboxState.currentGear = sData.mVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	sData.mVehicle.mEngineDriveState.gearboxState.targetGear = sData.mVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	//Set the vehicle to use the automatic gearbox.
	sData.mVehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

	//Set up the simulation context.
	//The snippet is set up with
	//a) z as the longitudinal axis
	//b) x as the lateral axis
	//c) y as the vertical axis.
	//d) metres  as the lengthscale.
	sData.mVehicleSimContext.setToDefault();
	
	sData.mVehicleSimContext.frame.lngAxis = PxVehicleAxes::ePosZ;
	sData.mVehicleSimContext.frame.latAxis = PxVehicleAxes::ePosX;
	sData.mVehicleSimContext.frame.vrtAxis = PxVehicleAxes::ePosY;
	sData.mVehicleSimContext.scale.scale = 1.0f;

	sData.mVehicleSimContext.gravity = game.physics->gGravity;
	sData.mVehicleSimContext.physxScene = game.physics->gScene;


	// mVehicleSimContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;

	EcsEntity sparkEntity = game.coordinator->createEntity();

	game.coordinator->addComponent(sparkEntity, SparkControls());
	game.coordinator->addComponent(sparkEntity, sData);
	game.coordinator->addComponent(sparkEntity, Transform());
	game.coordinator->addComponent(sparkEntity, rBody);
	game.coordinator->addComponent(sparkEntity, Model("assets/spark.obj"));

	dbug::log("GAME", 0, "Creating a new spark (ID:%d)", sparkEntity);


	


	// update the car as a test. it works perfectly fine here for some reason
	{
		auto &sd2 = game.coordinator->getComponent<SparkData>(sparkEntity);
		dbug::log("Pain", 2, "sd:%p", &sd2);
		dbug::log("Pain", 2, "rb:%p", &sData.mVehicle.mPhysXState.physxActor.rigidBody);

		sd2.mVehicle.mComponentSequence.setSubsteps(
			sd2.mVehicle.mComponentSequenceSubstepGroupHandle, 2);
		sd2.mVehicle.step(0.02, sd2.mVehicleSimContext);
		dbug::log("Pain", 2, "why");
	}
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
