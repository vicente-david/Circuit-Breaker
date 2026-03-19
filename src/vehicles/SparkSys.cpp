#include "SparkSys.h"
#include "debugUtils/Panel.h"
#include "graphics/Model.h"
#include "world/LapSystem.h"

void SparkSys::updateSparks(double dt, GameState &game) {
	
	sparkCollision(game);
	wallCollision(game);
	healZoneCheck(game, dt);
	
	bool reload = false;
	bool isP1 = true;
	for (const Entity &entity : entities) {
		SparkData &sData = game.coordinator->getComponent<SparkData>(entity);
		SparkControls &sControls = game.coordinator->getComponent<SparkControls>(entity);

		const PxVec3 linVel = sData.rBody->getLinearVelocity();
		const PxVec3 forwardDir = sData.rBody->getGlobalPose().q.getBasisVector2();

		sData.speed = linVel.magnitude();
		const PxU8 nbSubsteps = (sData.speed < 5.0f ? 3 : 1);

		if (sData.health > 0) // cant do anything if your dead lol
			sparkInputs(sData, sControls, dt);

		if (isP1) {
			dbug::log("INPUT", -1, "Spark commands: th: %f, brk: %f, trn: %f",
				sControls.throttle, sControls.brake, sControls.steering);
			dbug::log("ROLL", 0, "%f\tSTEER: %f", sData.rBody->getAngularVelocity().x, sControls.steering);
			isP1 = false;
		}
		// Respawn
		if (sControls.reset || sData.health <= 0) {
			sparkValuesReset(sData);
			respawnSpark(sData.rBody, getRespawnPose(entity, game));
		}

		// TODO: Put in helper (boost regen based off drift angle)
		// in degrees
		float driftAngle =
			PxAcos(linVel.getNormalized().dot(forwardDir.getNormalized())) *
			(180 / PxPi);
		driftAngle = std::abs(driftAngle);

		// regen boost if you're drifting
		float maxBoost = 100 - sData.health;
		if (entity == 2)
			dbug::log("GAME", -1, "player drift angle:%.2f, vel:%.2f",
					  driftAngle, linVel.magnitude());

		if (driftAngle > 20 && linVel.magnitude() > 10) {
			// how 'hard' of a drift (90 degrees gives full regen, 0 degrees
			// gives none)
			// cap at 90 degrees
			if (driftAngle > 90) {
				driftAngle = 90;
			}
			float multi = driftAngle / 90.f;
			if (entity == 2)
				dbug::log("GAME", -1, "drifin' (mulit:%.2f)", multi);

			sData.currBoost += sData.boostRegenSpeed * dt * multi;
			if (sData.currBoost > maxBoost) {
				sData.currBoost = maxBoost;
			}
		}

		// TODO: Compare with helper (shimmy)
		// shimmying
		if (sData.shimmyTimer <= 0) {
			if (controls.shimmyL) {
				dbug::log("GAME", 0, "slide to the left");
				shimmy(rBody, sData, false);
			}
		}

		// do the physx vehicle movement
		sData.mVehicle->mComponentSequence.setSubsteps(
			sData.mVehicle->mComponentSequenceSubstepGroupHandle, nbSubsteps);
		sData.mVehicle->step(dt, sData.mVehicleSimContext);

		// TODO: Put in helper (audio stuff)
		// update sound
		auto &sound = game.coordinator->getComponent<Sound>(entity);
		auto pos = rBody->getGlobalPose().p;
		rBody->getLinearVelocity();
		sound.position = glm::vec3(pos.x, pos.y, pos.z);
		auto vel = rBody->getGlobalPose().p;
		sound.position = glm::vec3(vel.x, vel.y, vel.z);

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

	sData.rBody = sData.mVehicle->mPhysXState.physxActor.rigidBody;

	// Adds to the shape of the chassis
	{
		PxBoxGeometry rearBoxGeom(PxVec3(0.85f, 0.25f, 0.2f));
		PxShape* rearBox = game.physics->gPhysics->createShape(rearBoxGeom, *game.physics->gMaterial, true);
		PxTransform rearBoxLocalPose(PxVec3(0.0f, 0.0f, -0.2f), PxQuat(PxIdentity));

		PxBoxGeometry midBoxGeom(PxVec3(0.6f, 0.25f, 0.1f));
		PxShape* midBox = game.physics->gPhysics->createShape(midBoxGeom, *game.physics->gMaterial, true);
		PxTransform midBoxLocalPose(PxVec3(0.0f, 0.0f, 0.1f), PxQuat(PxIdentity));

		rearBox->setLocalPose(rearBoxLocalPose);
		sData.rBody->attachShape(*rearBox);
		rearBox->release();

		midBox->setLocalPose(midBoxLocalPose);
		sData.rBody->attachShape(*midBox);
		midBox->release();

		PxReal newMass = sData.mVehicle->mBaseParams.rigidBodyParams.mass;
		PxRigidBodyExt::updateMassAndInertia(*sData.rBody, newMass);
	}
	// collision box to detect heal zones/if youre grounded
	{
		PxBoxGeometry groundBoxGeom(PxVec3(0.6f, 0.2f, 0.3f));
		PxShape *groundBox = game.physics->gPhysics->createShape(groundBoxGeom, *game.physics->gMaterial, true);
		PxTransform groundBoxLocalPose(PxVec3(0.0f, -0.5f, 0.1f), PxQuat(PxIdentity));

		groundBox->setLocalPose(groundBoxLocalPose);
		sData.rBody->attachShape(*groundBox);
		groundBox->release();
	}

	// Create vehicle filter
	PxFilterData chassisFilter(COLLISION_FLAG_CHASSIS,
							   COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
	// wheels have no collision
	PxFilterData tireFilter(COLLISION_FLAG_WHEEL, 0, 0, 0);
	PxFilterData groundFilter(COLLISION_FLAG_SPARK_GROUND,
							  COLLISION_FLAG_SPARK_GROUND_AGAINST, 0, 0);
	// PxFilterData tireFilter(0, 0, 0, 0);
	// Set flags
	PxU32 shapes = sData.rBody->getNbShapes();
	for (PxU32 i = 0; i < shapes; i++) {
		PxShape *shape = NULL;
		sData.rBody->getShapes(&shape, 1, i);

		// add filter to tires/chasis depending on type
		if (shape->getGeometry().getType() == physx::PxGeometryType::eBOX) {
			shape->setSimulationFilterData(chassisFilter);
		} else {
			shape->setSimulationFilterData(tireFilter);
		}

		// special ground trigger
		if (i == 7) {
			shape->setSimulationFilterData(groundFilter);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			shape->setFlag(PxShapeFlag::eVISUALIZATION, true);
		} else {

			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
			shape->setFlag(PxShapeFlag::eVISUALIZATION, true);
		}
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
	sData.rBody->userData = &sData.physData;

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
	game.coordinator->addComponent(sparkEntity, sData.rBody);
	if (sparkEntity == 3) {
		game.coordinator->addComponent(sparkEntity, Model("assets/spark2.obj"));
	}
	else
		game.coordinator->addComponent(sparkEntity, Model("assets/spark.obj"));

	// add engine sound
	Sound sound = game.audio->createSound("engine");
	sound.position = glm::vec3(startP.x, startP.y, startP.z);
	sound.start();
	game.coordinator->addComponent(sparkEntity, sound);

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
	sig.set(coord->getComponentType<physx::PxRigidBody *>());
	sig.set(coord->getComponentType<Sound>());
	sig.set(coord->getComponentType<LapCounter>());
	coord->setSystemSignature<SparkSys>(sig);

	return system;
}

// FLAG CHECKS
void SparkSys::sparkCollision(GameState& game) {
		for (auto const &colData : game.physics->callbacks->sparkSparkCol) {
		auto &sData1 = game.coordinator->getComponent<SparkData>(colData.spark1Id);
		auto &sData2 = game.coordinator->getComponent<SparkData>(colData.spark2Id);

		// don't do damage from hitting each other if sliding or boosting
		// Spark 1 logic
		if (sData1.shimmyTimer < 0.5 && !sData1.isBoosting)
			sData1.health -= colData.magnitude;
		//else
		//	dbug::log("GAME", 0, "i:%d Block!", colData.spark1Id);

		// Spark 2 logic
		if (sData2.shimmyTimer < 0.5 && !sData2.isBoosting)
			sData2.health -= colData.magnitude;
		//else
		//	dbug::log("GAME", 0, "i:%d Block!", colData.spark2Id);

		//dbug::log("GAME", 0, "i1:%d i2:%d Hit a car!", colData.spark1Id, colData.spark2Id);
	}
}

void SparkSys::wallCollision(GameState &game) {
	for (auto const& colData : game.physics->callbacks->sparkWallCol) {
		auto& sData =game.coordinator->getComponent<SparkData>(colData.sparkId);

		sData.health -= colData.magnitude * 0.75; // TODO: maybe dont hardcode damping value?
		dbug::log("GAME", 0, "Hit a wall!");
	}
}

void SparkSys::healZoneCheck(GameState& game, double dt) {
	for (auto const& colData : game.physics->callbacks->healingSparks) {
		auto& sData = game.coordinator->getComponent<SparkData>(colData);
		
		if (sData.health < sData.maxHealth) {
			sData.health += 10 * dt; // TODO: add regen rate to spark data
		
			if (sData.health > sData.maxHealth)
				sData.health = sData.maxHealth;
		}
	}
}

// COMMANDS
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
	stabilizeSpark(sData, sControls);
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

// FEATURES
void SparkSys::updateMaxBoost(SparkData& sData) {
	sData.maxBoost = sData.maxHealth - sData.health;
	if (sData.boost > sData.maxBoost)
		sData.boost = sData.maxBoost;
}

void SparkSys::applyBoost(SparkData& sData, bool useHealth, double dt) {
	const PxVec3 forwardVector = sData.rBody->getGlobalPose().q.getBasisVector2();
	
	if (sData.health <= 1)
		return;

	if (useHealth)
		sData.health -= sData.boostUseRate * dt * 0.5f; // use half the boost usage rate
	else
		sData.boost -= sData.boostUseRate * dt;

	// don't kill yourself from boosting
	if (sData.health <= 1)
		sData.health = 1;
	
	sData.rBody->addForce(forwardVector * sData.boostStrength, PxForceMode::eACCELERATION);
}

void SparkSys::boost(SparkData& sData, SparkControls& sControls, double dt) {
	updateMaxBoost(sData);

	// MAYBE TODO: change to a small delay before applying bigger boost (leave alone for now)
	// stop boosting if we've run out of normal boost
	if (sData.boost <= 0 && !sControls.boostWithHealth) {
		sData.isBoosting = false;
		//dbug::log("GAME", -1, "No boost!");
		return;
	}

	if (sControls.boost) {
		//dbug::log("GAME", -1, "boosting!");
		applyBoost(sData, sControls.boostWithHealth, dt);
		sData.isBoosting = true;
	}
	else if (sData.boost < sData.maxBoost) {
		// Case where boost > maxBoost is in updateMaxBoost since it always gets called before applying any boost
		sData.boost += sData.boostRegenRate * dt;
		sData.isBoosting = false;
	}
}

void SparkSys::applyShimmy(SparkData& sData, bool moveRight) {
	const PxVec3 lateralVector = sData.rBody->getGlobalPose().q.getBasisVector0();
	
	int flip = moveRight ? -1 : 1;
	
	sData.rBody->addForce(lateralVector * sData.shimmyForce * flip, PxForceMode::eVELOCITY_CHANGE);

	sData.shimmyTimer = sData.ShimmyCooldown;
}

void SparkSys::shimmy(SparkData& sData, SparkControls& sControls, double dt) {
	if (sData.shimmyTimer <= 0) {
		if (sControls.shimmyL) {
			//dbug::log("GAME", 0, "slide to the left");
			applyShimmy(sData, false);
		}

		if (sControls.shimmyR) {
			//dbug::log("GAME", 0, "slide to the right");
			applyShimmy(sData, true);
		}
	}
	else {
		sData.shimmyTimer -= dt;
	}

}

// STABILIZERS
void SparkSys::changeWheelParams(SparkData& sData, PxReal friction, PxReal latFriction, PxReal maxSteerAngle) {
	for (int i = 0; i < 4; i++) {
		sData.mVehicle->mBaseParams.tireForceParams[i].frictionVsSlip[2][1] = friction;
		sData.mVehicle->mBaseParams.tireForceParams[i].latStiffY = latFriction;
	}
	sData.mVehicle->mBaseParams.steerResponseParams.maxResponse = maxSteerAngle;
}

void SparkSys::driftStabilizer(SparkData& sData, SparkControls& sControls) {

	const PxVec3 linVel = sData.rBody->getLinearVelocity();
	const PxVec3 forward = sData.rBody->getGlobalPose().q.getBasisVector2();
	const PxVec3 lateral = sData.rBody->getGlobalPose().q.getBasisVector0();
	const PxVec3 lateralDir = lateral.getNormalized();

	const float lateralSpeed = linVel.dot(lateral);

	int ccw = PxSign(sData.rBody->getAngularVelocity().y); // rotating ccw = 1 and cw = -1
	int steerCcw = PxSign(sControls.steering); // steering ccw = 1 and cw = -1

	float driftCurve = 2.1f * sControls.steering;
	float angleCurve = 1.4f * sControls.steering;
	float yawVel = sData.rBody->getAngularVelocity().y;

	// Opposite forces automatically apply due to opposite sign when counter-steering
	PxVec3 driftDir = (forward + lateral * driftCurve);
	driftDir.y = 0.f;
	driftDir.normalize();
	float forceStrength = ccw == steerCcw ? 600.f : 1400.f;
	sData.rBody->addForce(driftDir * forceStrength);

	float torqueStrength = ccw == steerCcw ? 200.f : 800.f;
	sData.rBody->addTorque(PxVec3(0.f, 1.f, 0.f) * angleCurve * torqueStrength);

	float gripStrength = 240.f;
	sData.rBody->addForce(-lateralDir * lateralSpeed * gripStrength * yawVel);

	float rollDamping = sData.speed * 0.4f;
	sData.rBody->addTorque(PxVec3(1.f, 0.f, 0.f) * -sData.rBody->getAngularVelocity().x * rollDamping, PxForceMode::eACCELERATION);
}

void SparkSys::yawStabilizer(SparkData& sData) {
	const float yawVel = sData.rBody->getAngularVelocity().y;
	const float yawDamping = sData.speed * 0.15f;
	PxVec3 yawCorrection(0.f, -yawVel * yawDamping, 0.f);

	sData.rBody->addTorque(yawCorrection, PxForceMode::eACCELERATION);
}

void SparkSys::stabilizeSpark(SparkData& sData, SparkControls& sControls) {
	// TODO: drift state only when moving fast enough
	if (sControls.handbrake && sData.speed >= 15) {
		if (!sData.inDrift)
			changeWheelParams(sData, 3.8, 105600, PxDegToRad(30));

		sData.inDrift = true;
		driftStabilizer(sData, sControls); // Helps control oversteer
	}
	else {
		// Reset friction params to original values from JSON
		if (sData.inDrift)
			changeWheelParams(sData, 3.8, 145600, PxDegToRad(45));

		sData.inDrift = false;
		yawStabilizer(sData); // Helps prevent oversteer
	}
}

// RESPAWN
void SparkSys::respawnSpark(PxRigidBody* rBody, PxTransform respawnPose) {
	dbug::log("GAME", 0, "resetting");

	rBody->setGlobalPose(respawnPose);

	PxRigidDynamic* dBody = rBody->is<PxRigidDynamic>();
	dBody->setLinearVelocity(PxVec3(PxIdentity));
	dBody->setAngularVelocity(PxVec3(PxIdentity));
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

void SparkSys::sparkValuesReset(SparkData& sData) {
	sData.health = sData.maxHealth;
	sData.maxBoost = 0.0f;
	sData.boost = sData.maxBoost;
	sData.shimmyTimer = 0;
	sData.speed = 0.0f;
	sData.inReverse = false;
	sData.inDrift = false;
	sData.isBoosting = false;
}