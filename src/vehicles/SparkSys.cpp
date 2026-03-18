#include "SparkSys.h"
#include "debugUtils/Panel.h"
#include "graphics/Model.h"
#include "world/LapSystem.h"

void SparkSys::updateSparks(double dt, GameState &game) {
	
	sparkCollision(game);
	wallCollision(game);

	bool reload = false;
	bool isP1 = true;
	for (const Entity &entity : entities) {
		PxRigidBody *rBody = game.coordinator->getComponent<PxRigidBody *>(entity);
		SparkData &sData = game.coordinator->getComponent<SparkData>(entity);
		SparkControls &sControls = game.coordinator->getComponent<SparkControls>(entity);

		const PxVec3 linVel = rBody->getLinearVelocity();
		const PxVec3 forwardDir = rBody->getGlobalPose().q.getBasisVector2();

		sData.speed = linVel.magnitude();
		const PxU8 nbSubsteps = (sData.speed < 5.0f ? 3 : 1);

		sparkInputs(sData, sControls, dt);

		if (isP1) {
			dbug::log("INPUT", -1, "Spark commands: th: %f, brk: %f, trn: %f",
				sControls.throttle, sControls.brake, sControls.steering);
			dbug::log("ROLL", 0, "%f\tSTEER: %f", rBody->getAngularVelocity().x, sControls.steering);
			isP1 = false;
		}
		// Respawn
		if (sControls.reset)
			respawnSpark(rBody, getRespawnPose(entity, game));

		// do the physx vehicle movement
		sData.mVehicle->mComponentSequence.setSubsteps(
			sData.mVehicle->mComponentSequenceSubstepGroupHandle, nbSubsteps);
		sData.mVehicle->step(dt, sData.mVehicleSimContext);

		reload = sControls.reload;

		dbugPanel::sparkInfo(entity, sData.health, sData.boost);
	}

	// reload the tuning stuff from debug panel
	if (dbugPanel::tuning::reloadSpark || reload) {
		reloadSparkParams(game);
	}
}

Entity SparkSys::createSpark(GameState &game, PxVec3 startP) {

	Entity sparkEntity = game.coordinator->createEntity();

	// if you create a sparkdata object in this function it gets freed, so
	// we need to get a referenct from the ECS coordinator instead (this
	// defintely didn't take hours to debug. I love c)
	game.coordinator->addComponent(sparkEntity, SparkData());
	SparkData &sData = game.coordinator->getComponent<SparkData>(sparkEntity);
	sData.mVehicle = std::make_shared<EngineDriveVehicle>();

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
	sData.neutralGear = sData.mVehicle->mEngineDriveParams.gearBoxParams.neutralGear;
	sData.mVehicle->mEngineDriveState.gearboxState.currentGear = sData.neutralGear + 1;
	sData.mVehicle->mEngineDriveState.gearboxState.targetGear = sData.neutralGear + 1;
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

	// TODO: make sure this works, applying logitudinally would be nice too
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
	dbugPanel::tuning::setFolder = false;
	
	for (auto const &entity : entities) {
		auto &sData = game.coordinator->getComponent<SparkData>(entity);
		sData.mVehicleDataPath = dbugPanel::tuning::configFolder.c_str();
	}

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


// ================================ HELPER FUNCTIONS ================================

std::shared_ptr<SparkSys> SparkSys::registerSystem(std::shared_ptr<Coordinator> &coord) {
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

void SparkSys::sparkCollision(GameState& game) {
	for (auto const& pair : game.physics->callbacks->sparkSparkCol) {
		auto& sData1 = game.coordinator->getComponent<SparkData>(pair.first);
		auto& rBody1 = game.coordinator->getComponent<PxRigidBody*>(pair.first);
		auto& sData2 = game.coordinator->getComponent<SparkData>(pair.second);
		auto& rBody2 = game.coordinator->getComponent<PxRigidBody*>(pair.second);

		auto velDiff = rBody1->getLinearVelocity() - rBody2->getLinearVelocity();

		sData2.health -= velDiff.magnitude();
		sData1.health -= velDiff.magnitude();

		dbug::log("GAME", 0, "i1:%d i2:%d Hit a car!", pair.first, pair.second);

	}
}

void SparkSys::wallCollision(GameState &game) {
	for (auto const& entity : game.physics->callbacks->sparkWallCol) {
		auto& sData = game.coordinator->getComponent<SparkData>(entity);
		auto& rBody = game.coordinator->getComponent<PxRigidBody*>(entity);
		sData.health -= rBody->getLinearVelocity().magnitude();
		dbug::log("GAME", 0, "Hit a wall!");
		printf("!\n");

	}
}

void SparkSys::sparkInputs(SparkData &sData, SparkControls &sControls, double dt) {

	sData.mVehicle->mCommandState.brakes[0] = sControls.brake;
	sData.mVehicle->mCommandState.brakes[1] = sControls.handbrake;
	sData.mVehicle->mCommandState.nbBrakes = 1;
	sData.mVehicle->mCommandState.throttle = sControls.throttle;
	sData.mVehicle->mCommandState.steer = sControls.steering;

	// Check for reverse 
	reverse(sData, sControls);

	// boosting
	boost(sData, sControls, dt);

	// shimmying
	shimmy(sData, sControls, dt);

	// Stabilizers
	{
		auto rBody = sData.mVehicle->mPhysXState.physxActor.rigidBody;
		
		const PxVec3 linVel = rBody->getLinearVelocity();
		const PxVec3 forward = rBody->getGlobalPose().q.getBasisVector2();
		const PxVec3 lateral = rBody->getGlobalPose().q.getBasisVector0();
		//const PxVec3 vertical = rBody->getGlobalPose().q.getBasisVector1();

		//const PxVec3 travelDir = linVel.getNormalized();
		//const PxVec3 forwardDir = forward.getNormalized();
		const PxVec3 lateralDir = lateral.getNormalized();
		//const PxVec3 verticalDir = vertical.getNormalized();

		////// Direction vectors projected on to the xz-plane
		//const PxVec3 travelDirXZ = (travelDir - (travelDir.y * PxVec3(0.f, 1.f, 0.f))).getNormalized();
		//const PxVec3 ForwardDirXZ = (forwardDir - (forwardDir.y * PxVec3(0.f, 1.f, 0.f))).getNormalized();
		//const PxVec3 lateralDirXZ = (lateralDir - (lateralDir.y * PxVec3(0.f, 1.f, 0.f))).getNormalized();

		//const float forwardSpeed = linVel.dot(forward);
		const float lateralSpeed = linVel.dot(lateral);

		//const float driftAmount = sData.speed != 0.f ? lateralSpeed / forwardSpeed : 0.f; // in radians
		//const float driftAngle = PxAbs(driftAmount) * 90.f;

		if (sControls.handbrake) {
			if (!sData.inDrift) {
				for (int i = 0; i < 4; i++) {
					sData.mVehicle->mBaseParams.tireForceParams[i].frictionVsSlip[2][1] = 2.4;
					sData.mVehicle->mBaseParams.tireForceParams[i].latStiffY = 105600;
				}
				sData.mVehicle->mBaseParams.steerResponseParams.maxResponse = 0.523599029f;
			}
			sData.inDrift = true;
			const int ccw = PxSign(rBody->getAngularVelocity().y); // rotating ccw = 1 and cw = -1
			const int steerCcw = PxSign(sControls.steering); // steering ccw = 1 and cw = -1

			
			const float driftCurve =  2.1f * sControls.steering;
			const float angleCurve =  1.4f * sControls.steering;
			const float yawVel = rBody->getAngularVelocity().y;

			// Opposite forces automatically apply due to opposite sign when counter-steering
			PxVec3 driftDir = (forward + lateral * driftCurve);
			const float forceStrength = ccw == steerCcw ? 600.f : 1400.f;
			rBody->addForce(driftDir * forceStrength);

			const float torqueStrength = ccw == steerCcw ? 200.f : 600.f;
			rBody->addTorque(PxVec3(0.f, 1.f, 0.f) * angleCurve * torqueStrength);

			const float gripStrength = 240.f;
			rBody->addForce(-lateralDir * lateralSpeed * gripStrength * yawVel);

			const float rollDamping = sData.speed * 0.4f;
			rBody->addTorque(PxVec3(0.f, 0.f, 1.f) * -rBody->getAngularVelocity().x * rollDamping);
		}
		else {
			// Reset friction params to original values from JSON
			if (sData.inDrift) {
				for (int i = 0; i < 4; i++) {
					sData.mVehicle->mBaseParams.tireForceParams[i].frictionVsSlip[2][1] = 3.8;
					sData.mVehicle->mBaseParams.tireForceParams[i].latStiffY = 145600;
				}
				sData.mVehicle->mBaseParams.steerResponseParams.maxResponse = 0.785398163f;
			}
			sData.inDrift = false;
			
			//sData.forwardDir = forwardDir;

			// Helps prevent spin-outs
			const float yawVel = rBody->getAngularVelocity().y;
			float yawDamping = sData.speed * 0.15f;
			PxVec3 yawCorrection(0.f, -yawVel * yawDamping, 0.f);
			rBody->addTorque(yawCorrection, PxForceMode::eACCELERATION);
		}
	}
}

void SparkSys::reverse(SparkData& sData, SparkControls& sControls) {
	// Must be basically stopped or already in reverse
	if ((sData.speed < 0.1f || sData.inReverse) && sControls.brake) {
		sData.mVehicle->mCommandState.brakes[0] = sControls.throttle; // Brake becomes throttle
		sData.mVehicle->mCommandState.throttle = sControls.brake; // Throttle becomes brake
		sData.mVehicle->mEngineDriveState.gearboxState.currentGear = sData.neutralGear - 1; // Shifts to reverse
		sData.inReverse = true;
	}
	// By necessity this can only happen if brake is fully released
	else if (sControls.throttle) {
		// Shifts into 1st throttle is applied immediately
		if (sData.mVehicle->mEngineDriveState.gearboxState.currentGear <= sData.neutralGear) {
			sData.mVehicle->mEngineDriveState.gearboxState.currentGear = sData.neutralGear + 1;
		}
		sData.inReverse = false;
	}
}

void SparkSys::updateMaxBoost(SparkData& sData) {
	sData.maxBoost = sData.maxHealth - sData.health;
}

void SparkSys::applyBoost(SparkData& sData) {
	PxRigidBody* rBody = sData.mVehicle->mPhysXState.physxActor.rigidBody;
	const PxVec3 forwardVector = rBody->getGlobalPose().q.getBasisVector2();
	
	sData.boost -= sData.boostUseRate;

	rBody->addForce(forwardVector * sData.boostStrength, PxForceMode::eACCELERATION);
}

void SparkSys::boost(SparkData& sData, SparkControls& sControls, double dt) {
	updateMaxBoost(sData);

	if (sControls.boost && sData.boost > 0 && sData.inDrift) {
		dbug::log("GAME", -1, "boosting!");
		applyBoost(sData);
	}
	else if (sData.boost < sData.maxBoost) {
		sData.boost += sData.boostRegenRate * dt;

		if (sData.boost > sData.maxBoost) {
			sData.boost = sData.maxBoost;
			dbug::log("GAME", 0, "boost full");
		}
	}
}

void SparkSys::applyShimmy(SparkData& sData, bool moveRight) {
	PxRigidBody* rBody = sData.mVehicle->mPhysXState.physxActor.rigidBody;
	const PxVec3 lateralVector = rBody->getGlobalPose().q.getBasisVector0();
	
	int flip = moveRight ? -1 : 1;
	
	rBody->addForce(lateralVector * sData.shimmyForce * flip, PxForceMode::eVELOCITY_CHANGE);

	sData.shimmyTimer = sData.ShimmyCooldown;
}

void SparkSys::shimmy(SparkData& sData, SparkControls& sControls, double dt) {
	if (sData.shimmyTimer <= 0) {
		if (sControls.shimmyL) {
			dbug::log("GAME", 0, "slide to the left");
			applyShimmy(sData, false);
		}

		if (sControls.shimmyR) {
			dbug::log("GAME", 0, "slide to the right");
			applyShimmy(sData, true);
		}
	}
	else {
		sData.shimmyTimer -= dt;
	}

}

void SparkSys::respawnSpark(PxRigidBody* rBody, PxTransform respawnPose) {
	dbug::log("GAME", 0, "resetting");

	rBody->setGlobalPose(respawnPose);

	PxRigidDynamic* dynamicBody = rBody->is<PxRigidDynamic>();
	dynamicBody->setLinearVelocity(PxVec3(PxIdentity));
	dynamicBody->setAngularVelocity(PxVec3(PxIdentity));
}

PxTransform SparkSys::getRespawnPose(Entity entity, GameState& game) {
	// copied logic from RespawnSystem::update
	LapCounter& prog = game.coordinator->getComponent<LapCounter>(entity);

	glm::vec3 p = prog.lastCheckpointPos;
	PxVec3 pos(p.x, p.y + 2, p.z);

	glm::vec3 q = prog.lastCheckpointDir;
	PxQuat quat(PxAtan2(q.x, q.z), PxVec3(0.f, 1.f, 0.f));

	return PxTransform(pos, quat);
}