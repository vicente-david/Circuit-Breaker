
#include "vehicles/SparkSys.h"
#include "GameState.h"
#include "PxActor.h"
#include "PxForceMode.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"
#include "PxShape.h"
#include "SparkComponents.h"
#include "debugUtils/Logger.h"
#include "debugUtils/Panel.h"
#include "ecs/Component.h"
#include "ecs/EntityManager.h"
#include "geometry/PxGeometry.h"
#include "graphics/Model.h"
#include "physics/CollisionData.h"
#include "physics/PhysicsManager.h"
#include <cstdio>
#include <memory>

void SparkSys::updateSparks(double dt, GameState &game) {
	for (auto const &pair : game.physics->callbacks->sparkSparkCol) {
		auto &sData1 = game.coordinator->getComponent<SparkData>(pair.first);
		auto &rBody1 = game.coordinator->getComponent<PxRigidBody *>(pair.first);
		auto &sData2 = game.coordinator->getComponent<SparkData>(pair.second);
		auto &rBody2 = game.coordinator->getComponent<PxRigidBody *>(pair.second);

		auto velDiff = rBody1->getLinearVelocity() - rBody2->getLinearVelocity();

		sData2.health-=velDiff.magnitude();
		sData1.health-=velDiff.magnitude();

		dbug::log("GAME", 0, "i1:%d i2:%d Hit a car!", pair.first, pair.second);

	}
	for (auto const &entity : game.physics->callbacks->sparkWallCol) {
		auto &sData = game.coordinator->getComponent<SparkData>(entity);
		auto &rBody = game.coordinator->getComponent<PxRigidBody *>(entity);
		sData.health-=rBody->getLinearVelocity().magnitude();
		dbug::log("GAME", 0, "Hit a wall!");
		printf("!\n");

	}
	bool reload = false;
	for (auto const &entity : entities) {
		auto &rBody = game.coordinator->getComponent<PxRigidBody *>(entity);
		auto &sData = game.coordinator->getComponent<SparkData>(entity);
		auto &controls = game.coordinator->getComponent<SparkControls>(entity);

		const PxVec3 linVel = rBody->getLinearVelocity();
		const PxVec3 forwardDir = rBody->getGlobalPose().q.getBasisVector2();

		const PxReal speed = linVel.dot(forwardDir);
		const PxU8 nbSubsteps = (speed < 5.0f ? 3 : 1);
		sData.speed = speed;

		sData.mVehicle->mCommandState.brakes[0] = controls.brake;
		sData.mVehicle->mCommandState.brakes[1] = controls.handbrake;
		sData.mVehicle->mCommandState.nbBrakes = 2;
		sData.mVehicle->mCommandState.throttle = controls.throttle;
		sData.mVehicle->mCommandState.steer = controls.steering;

		dbug::log("INPUT", -1, "Spark commands: th: %f, brk: %f, trn: %f",
				  controls.throttle, controls.brake, controls.steering);
		
		// Check for reverse (brake + throttle when stopped)
		if (speed < 0.1f && controls.brake && controls.throttle) {
			sData.mVehicle->mCommandState.brakes[0] = 0.f;
			sData.mVehicle->mEngineDriveState.gearboxState.currentGear =
				sData.mVehicle->mEngineDriveParams.gearBoxParams.neutralGear - 1;
		}

		// Apply handbrake
		sData.mVehicle->mCommandState.brakes[1] = controls.handbrake;

		// boosting
		if (controls.boost && sData.currBoost > 0) {
			dbug::log("GAME", -1, "boosting!");
			boost(rBody, sData);
		} else if (sData.currBoost < 100) {
			sData.currBoost += sData.boostRegenSpeed * dt;

			if (sData.currBoost > 100) {
				sData.currBoost = 100;
				dbug::log("GAME", 0, "boost full");
			}
		}

		// shimmying
		if (sData.shimmyTimer <= 0) {
			if (controls.shimmyL) {
				dbug::log("GAME", 0, "slide to the left");
				shimmy(rBody, sData, false);
			}

			if (controls.shimmyR) {
				dbug::log("GAME", 0, "slide to the right");
				shimmy(rBody, sData, true);
			}
		} else if (sData.shimmyTimer > 0) {
			sData.shimmyTimer -= dt;
		}
		if (controls.reset) {
			respawn(rBody);
		}

		// do the physx vehicle movement
		sData.mVehicle->mComponentSequence.setSubsteps(
			sData.mVehicle->mComponentSequenceSubstepGroupHandle, nbSubsteps);
		sData.mVehicle->step(dt, sData.mVehicleSimContext);
		// rBody->addForce(forwardDir *controls.throttle,
		// PxForceMode::eACCELERATION);
		reload = controls.reload;

		dbugPanel::sparkInfo(entity, sData.health, sData.currBoost);
	}

	// reload the tuning stuff from debug panel
	if (dbugPanel::tuning::reloadSpark || reload) {
		dbugPanel::tuning::setFolder = false;
		for (auto const &entity : entities) {
			auto &sData = game.coordinator->getComponent<SparkData>(entity);
			sData.mVehicleDataPath = dbugPanel::tuning::configFolder.c_str();
		}
		reloadSparkParams(game);
	}
}

void SparkSys::shimmy(PxRigidBody *rBody, SparkData &sData, bool rightDir) {
	const PxVec3 latDir = rBody->getGlobalPose().q.getBasisVector0();
	int flip = (rightDir) ? -1 : 1;
	float shimmyForce = 15.f;

	rBody->addForce(latDir * shimmyForce * flip, PxForceMode::eVELOCITY_CHANGE);
	dbug::log("GAME", 0, "Weeeeeeee!");

	sData.shimmyTimer = sData.ShimmyCooldown;
}

void SparkSys::boost(PxRigidBody *rBody, SparkData &sData) {
	const PxVec3 forwardDir = rBody->getGlobalPose().q.getBasisVector2();
	float boostStrength = 10.f;

	sData.currBoost -= 0.5f;

	rBody->addForce(forwardDir * boostStrength, PxForceMode::eACCELERATION);
}

void SparkSys::respawn(PxRigidBody *rBody) {
	dbug::log("GAME", 0, "resetting");

	rBody->setGlobalPose(
		PxTransform(PxVec3(0.f, 1.f, -50.f), PxQuat(PxIdentity)));

	PxRigidDynamic *dynamicBody = rBody->is<PxRigidDynamic>();
	dynamicBody->setLinearVelocity(PxVec3(PxIdentity));
	dynamicBody->setAngularVelocity(PxVec3(PxIdentity));
}

Entity SparkSys::createSpark(GameState &game, PxVec3 startP) {

	Entity sparkEntity = game.coordinator->createEntity();

	// if you create a sparkdata object in this function it gets freed, so
	// we need to get a referenct from the ECS coordinator instead (this
	// defintely didn't take hours to debug. I love c)
	game.coordinator->addComponent(sparkEntity, SparkData());
	SparkData &sData = game.coordinator->getComponent<SparkData>(sparkEntity);
	sData.mVehicle = std::make_shared<CustomTireVehicle>();

	// SparkData sData;
	// Load the params from json or set directly.
	sData.mVehicleDataPath = dbugPanel::tuning::configFolder.c_str();

	readBaseParamsFromJsonFile(
		sData.mVehicleDataPath, 
		dbugPanel::tuning::basePath.c_str(), 
		sData.mVehicle->mBaseParams);
	readEngineDrivetrainParamsFromJsonFile(
		sData.mVehicleDataPath, 
		dbugPanel::tuning::enginePath.c_str(),
		sData.mVehicle->mEngineDriveParams);
	setPhysXIntegrationParams(
		sData.mVehicle->mBaseParams.axleDescription, sData.mMaterialFrictions,
		sData.mNbMaterialFrictions, sData.mDefaultMaterialFriction,
		sData.mVehicle->mPhysXParams);

	// Set the states to default.
	// Creates the first and main shape of the chassis and wheels
	if (!sData.mVehicle->initialize(
			*game.physics->gPhysics, PxCookingParams(PxTolerancesScale()),
			*game.physics->gMaterial,
			EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE)) {
		return -1;
	}

	// Apply a start pose to the physx actor and add it to the physx scene.
	PxTransform startPose(startP, PxQuat(PxIdentity));
	sData.mVehicle->setUpActor(*game.physics->gScene, startPose, sData.mVehicleName);

	auto rBody = sData.mVehicle->mPhysXState.physxActor.rigidBody;

	// Adds to the shape of the chassis
	{
		PxBoxGeometry rearBoxGeom(PxVec3(0.85f, 0.25f, 0.2f));
		PxShape* rearBox = game.physics->gPhysics->createShape(rearBoxGeom, *game.physics->gMaterial, true);
		PxTransform rearBoxLocalPose(PxVec3(0.0f, 0.0f, -0.2f), PxQuat(PxIdentity));

		PxBoxGeometry midBoxGeom(PxVec3(0.6f, 0.25f, 0.1f));
		PxShape* midBox = game.physics->gPhysics->createShape(midBoxGeom, *game.physics->gMaterial, true);
		PxTransform midBoxLocalPose(PxVec3(0.0f, 0.0f, 0.1f), PxQuat(PxIdentity));

		rearBox->setLocalPose(rearBoxLocalPose);
		rBody->attachShape(*rearBox);
		rearBox->release();

		midBox->setLocalPose(midBoxLocalPose);
		rBody->attachShape(*midBox);
		midBox->release();

		PxReal newMass = sData.mVehicle->mBaseParams.rigidBodyParams.mass;
		PxRigidBodyExt::updateMassAndInertia(*rBody, newMass);
	}

	// Create vehicle filter
	PxFilterData chassisFilter(COLLISION_FLAG_CHASSIS,
							   COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
	PxFilterData tireFilter(COLLISION_FLAG_WHEEL, COLLISION_FLAG_GROUND, 0, 0);
	// PxFilterData tireFilter(0, 0, 0, 0);
	// Set flags
	PxU32 shapes = rBody->getNbShapes();
	for (PxU32 i = 0; i < shapes; i++) {
		PxShape *shape = NULL;
		rBody->getShapes(&shape, 1, i);

		// add filter to tires/chasis depending on type
		if (shape->getGeometry().getType() == physx::PxGeometryType::eBOX) {
			shape->setSimulationFilterData(chassisFilter);
		} else {
			shape->setSimulationFilterData(tireFilter);
		}

		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
		shape->setFlag(PxShapeFlag::eVISUALIZATION, true);
	}

	// Set the vehicle in 1st gear.
	sData.mVehicle->mEngineDriveState.gearboxState.currentGear =
		sData.mVehicle->mEngineDriveParams.gearBoxParams.neutralGear + 1;
	sData.mVehicle->mEngineDriveState.gearboxState.targetGear =
		sData.mVehicle->mEngineDriveParams.gearBoxParams.neutralGear + 1;
	// Set the vehicle to use the automatic gearbox.
	sData.mVehicle->mTransmissionCommandState.targetGear =
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
	sData.physData.entity = sparkEntity;
	rBody->userData = &sData.physData;

	sData.mVehicleSimContext.gravity = game.physics->gGravity;
	sData.mVehicleSimContext.physxScene = game.physics->gScene;

	sData.mVehicleSimContext.physxActorUpdateMode =
		PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;
	
	// Tire sweeps instead of raycasts
	sData.mCylinderSweepMesh = PxVehicleUnitCylinderSweepMeshCreate(sData.mVehicleSimContext.frame,
		*game.physics->gPhysics, PxCookingParams(PxTolerancesScale()));
	
	if (!sData.mCylinderSweepMesh) 
		return false;
	sData.mVehicleSimContext.physxUnitCylinderSweepMesh = sData.mCylinderSweepMesh;

	//Larger lateral damping factor than default to avoid drift when nearly rest
	sData.mVehicleSimContext.tireStickyParams.stickyParams[PxVehicleTireDirectionModes::eLATERAL].damping = 1.0f;


	// SparkControls controls;
	game.coordinator->addComponent(sparkEntity, SparkControls());
	game.coordinator->addComponent(sparkEntity, sData);
	game.coordinator->addComponent(sparkEntity, Transform());
	game.coordinator->addComponent(sparkEntity, rBody);
	if (sparkEntity == 3) {
		game.coordinator->addComponent(sparkEntity, Model("assets/spark2.obj"));
	}
	else
		game.coordinator->addComponent(sparkEntity, Model("assets/spark.obj"));

	dbug::log("GAME", 0, "Creating a new spark (ID:%d)", sparkEntity);

	return sparkEntity;
}

// updates the drive params of all the active sparks
void SparkSys::reloadSparkParams(GameState &game) {
	dbug::log("GAME", 0, "Reloading spark config");
	for (auto const &entity : entities) {
		dbug::log("GAME", 0, "setting entity %d", entity);
		auto &sData = game.coordinator->getComponent<SparkData>(entity);
		 
		// load params for vehicle base
		const char *baseFileName = dbugPanel::tuning::basePath.c_str();
		readBaseParamsFromJsonFile(
			sData.mVehicleDataPath,
			baseFileName,
			sData.mVehicle->mBaseParams);
		// Changes the parameters of the engine
		const char *engineFileName = dbugPanel::tuning::enginePath.c_str();
		readEngineDrivetrainParamsFromJsonFile(
			sData.mVehicleDataPath, 
			engineFileName,
			sData.mVehicle->mEngineDriveParams);

		printf("scene scale %f, car scale:%f\n", game.physics->gPhysics->getTolerancesScale().length, sData.mVehicleSimContext.scale.scale);
	}
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
