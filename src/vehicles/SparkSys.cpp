#include "SparkSys.h"
#include "debugUtils/Panel.h"
#include "graphics/Model.h"
#include "world/LapSystem.h"

void SparkSys::updateSparks(double dt, GameState &game) {
	
	sparkCollision(game);
	wallCollision(game);
	healZoneCheck(game, dt);
	
	bool reload = false;
	for (const Entity &entity : entities) {
		SparkData &sData = game.coordinator->getComponent<SparkData>(entity);
		SparkControls &sControls = game.coordinator->getComponent<SparkControls>(entity);

		sData.speed = sData.rBody->getLinearVelocity().magnitude();
		const PxU8 nbSubsteps = (sData.speed < 5.0f ? 3 : 1);

		// TODO: flash "Short Circuit" on screen (like you're dead)
		checkDeath(sData, dt);

		if (!sData.isDead) // cant do anything if your dead lol
			sparkInputs(sData, sControls, dt);

		// Respawn
		respawn(sData, sControls, dt);

		// do the physx vehicle movement
		sData.mVehicle->mComponentSequence.setSubsteps(
			sData.mVehicle->mComponentSequenceSubstepGroupHandle, nbSubsteps);
		sData.mVehicle->step(dt, sData.mVehicleSimContext);

		// Check in air
		checkAirborne(sData, dt);

		reload = sControls.reload;

		dbugPanel::sparkInfo(entity, sData.health, sData.boost);
	}

	// reload the tuning stuff from debug panel
	if (dbugPanel::tuning::reloadSpark || reload) {
		reloadSparkParams(game);
	}
}

Entity SparkSys::createSpark(GameState &game, PxVec3 startP, std::string name) {

	Entity sparkEntity = game.coordinator->createEntity();

	// if you create a sparkdata object in this function it gets freed, so
	// we need to get a referenct from the ECS coordinator instead (this
	// defintely didn't take hours to debug. I love c)
	game.coordinator->addComponent(sparkEntity, SparkData());
	SparkData &sData = game.coordinator->getComponent<SparkData>(sparkEntity);
	sData.mVehicle = std::make_shared<EngineDriveVehicle>();

	// Spark needs a name
	sData.mVehicleName = name;

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
	sData.mVehicle->setUpActor(*game.physics->gScene, startPose, sData.mVehicleName.c_str());

	sData.rBody = sData.mVehicle->mPhysXState.physxActor.rigidBody;

	// Adds to the shape of the chassis
	{
		PxBoxGeometry rearBoxGeom(PxVec3(0.85f, 0.15f, 0.2f));
		PxShape* rearBox = game.physics->gPhysics->createShape(rearBoxGeom, *game.physics->gMaterial, true);
		PxTransform rearBoxLocalPose(PxVec3(0.0f, 0.1f, -0.2f), PxQuat(PxIdentity));

		PxBoxGeometry sideBoxGeom(PxVec3(0.1f, 0.15f, 0.5f));
		PxShape* leftSideBox = game.physics->gPhysics->createShape(sideBoxGeom, *game.physics->gMaterial, true);
		PxShape* rightSideBox = game.physics->gPhysics->createShape(sideBoxGeom, *game.physics->gMaterial, true);
		PxTransform leftSideBoxLocalPose(PxVec3(0.43f, 0.1f, 0.3f), PxQuat(0.924f, 0.0f, 0.383f, 0.0f));
		PxTransform rightSideBoxLocalPose(PxVec3(-0.43f, 0.1f, 0.3f), PxQuat(0.924f, 0.0f, -0.383f, 0.0f));

		rearBox->setLocalPose(rearBoxLocalPose);
		sData.rBody->attachShape(*rearBox);
		rearBox->release();

		leftSideBox->setLocalPose(leftSideBoxLocalPose);
		sData.rBody->attachShape(*leftSideBox);
		leftSideBox->release();

		rightSideBox->setLocalPose(rightSideBoxLocalPose);
		sData.rBody->attachShape(*rightSideBox);
		rightSideBox->release();

		PxReal newMass = sData.mVehicle->mBaseParams.rigidBodyParams.mass;
		PxRigidBodyExt::updateMassAndInertia(*sData.rBody, newMass);
	}
	// collision box to detect heal zones/if youre grounded
	{
		PxBoxGeometry groundRearBoxGeom(PxVec3(0.85f, 0.4f, 0.3f));
		PxShape *groundRearBox = game.physics->gPhysics->createShape(groundRearBoxGeom, *game.physics->gMaterial, true);
		PxTransform groundRearBoxLocalPose(PxVec3(0.0f, -0.1f, -0.1f), PxQuat(PxIdentity));

		PxBoxGeometry groundFrontBoxGeom(PxVec3(0.3f, 0.4f, 0.3f));
		PxShape* groundFrontBox = game.physics->gPhysics->createShape(groundFrontBoxGeom, *game.physics->gMaterial, true);
		PxTransform groundFrontBoxLocalPose(PxVec3(0.0f, -0.1f, 0.5f), PxQuat(PxIdentity));

		groundRearBox->setLocalPose(groundRearBoxLocalPose);
		sData.rBody->attachShape(*groundRearBox);
		groundRearBox->release();

		groundFrontBox->setLocalPose(groundFrontBoxLocalPose);
		sData.rBody->attachShape(*groundFrontBox);
		groundFrontBox->release();
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
		if (i == 8 || i == 9) {
			shape->setSimulationFilterData(groundFilter);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			shape->setFlag(PxShapeFlag::eVISUALIZATION, false); // can remove for performance
		} else {

			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
			shape->setFlag(PxShapeFlag::eVISUALIZATION, true); // can remove for performance
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

	sData.rBody->setMaxAngularVelocity(6.f);
	
	// SparkControls controls;
	game.coordinator->addComponent(sparkEntity, SparkControls());
	game.coordinator->addComponent(sparkEntity, Transform());
	game.coordinator->addComponent(sparkEntity, sData.rBody);

	if (sData.mVehicleName == "P2") {
		game.coordinator->addComponent(sparkEntity, Model("assets/spark2.obj"));
	}
	else if (sData.mVehicleName == "P3") {
		game.coordinator->addComponent(sparkEntity, Model("assets/spark3.obj"));
	}
	else if (sData.mVehicleName == "P4") {
		game.coordinator->addComponent(sparkEntity, Model("assets/spark4.obj"));
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

void SparkSys::checkDeath(SparkData& sData, double dt) {
	if (sData.health > 0)
		return;
	
	if (sData.isDead && sData.deathTimer > 0) {
		sData.deathTimer -= dt;
		return;
	}

	// don't want negative health for UI purposes
	if (sData.health < 0) 
		sData.health = 0; 
	
	sData.isDead = true;
}

void SparkSys::checkAirborne(SparkData& sData, double dt) {
	// do check after simulation step
	bool grounded = false; // assume you're airborne
	for (int i = 0; i < 4; i++) {
		// if 1 tire is has contact -> you are grounded
		grounded |= sData.mVehicle->mBaseState.roadGeomStates[i].hitState;
	}

	// TODO: find a way to detect if you're grounded but just flipped over 
	if (grounded) 
		sData.offGroundTimer = sData.offGroundLimit;

	else if (sData.offGroundTimer > 0)
			sData.offGroundTimer -= dt;

	if (!sData.isGrounded && grounded) // just touched ground
		angularResistance(sData); // pre-maturely kill angResTimer

	sData.isGrounded = grounded;
}

void SparkSys::angularResistance(SparkData& sData, PxReal val, double duration) {
	sData.rBody->setAngularDamping(val);
	sData.angResTimer = duration; // duration of zero means indefinitely
}

void SparkSys::checkAngResistace(SparkData& sData, double dt) {
	if (sData.angResTimer <= 0)
		return;
	
	sData.angResTimer -= dt;
	
	// restore to default resistance
	if (sData.angResTimer <= 0) 
		angularResistance(sData);
}

// FLAG CHECKS
void SparkSys::sparkCollision(GameState& game) {
		for (auto const &colData : game.physics->callbacks->sparkSparkCol) {
		auto &sData1 = game.coordinator->getComponent<SparkData>(colData.spark1Id);
		auto &sData2 = game.coordinator->getComponent<SparkData>(colData.spark2Id);

		// don't do damage from hitting each other if sliding or boosting
		// Spark 1 logic
		if (sData1.shimmyTimer < sData1.shimmyInvincible && !sData1.isBoosting)
			sData1.health -= colData.magnitude;
		//else
		//	dbug::log("GAME", 0, "i:%d Block!", colData.spark1Id);

		// Spark 2 logic
		if (sData2.shimmyTimer < sData2.shimmyInvincible && !sData2.isBoosting)
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
		if (sData.health < 0)
			sData.health = 0;

		//dbug::log("GAME", 0, "Hit a wall!");
	}
}

void SparkSys::healZoneCheck(GameState& game, double dt) {
	for (auto const& colData : game.physics->callbacks->healingSparks) {
		auto& sData = game.coordinator->getComponent<SparkData>(colData);
		
		// Make sure spark is not healing while dead
		if (sData.health < sData.maxHealth && !sData.isDead) {
			sData.health += sData.healthRegenRate * dt;
		
			if (sData.health > sData.maxHealth)
				sData.health = sData.maxHealth;
		}
	}
}

// COMMANDS
void SparkSys::sparkInputs(SparkData &sData, SparkControls &sControls, double dt) {

	sData.mVehicle->mCommandState.brakes[0] = sControls.brake;
	sData.mVehicle->mCommandState.nbBrakes = 1;
	sData.mVehicle->mCommandState.throttle = sControls.throttle;
	sData.mVehicle->mCommandState.steer = sControls.steering;

	boost(sData, sControls, dt);
	shimmy(sData, sControls, dt);

	brake(sData, sControls); // stronger braking
	reverse(sData, sControls); // check for reverse

	if (!sData.isGrounded)
		return;

	checkAngResistace(sData, dt);
	sparkHandling(sData, sControls);
	
	regenBoost(sData, dt); // boost regeneration
	
}

void SparkSys::brake(SparkData& sData, SparkControls& sControls) {
	if (!sControls.brake)
		return;
	
	// stops arial rotation if brake is pressed
	if (!sData.isGrounded) {
		angularResistance(sData, PX_MAX_REAL, sData.offGroundLimit);
		return;
	}

	const PxVec3 linVel = sData.rBody->getLinearVelocity();
	float brakingForce = 100.f;
	sData.rBody->addForce(-linVel * brakingForce);
}

void SparkSys::reverse(SparkData& sData, SparkControls& sControls) {
	// Must be basically stopped or already in reverse
	if ((sData.speed < 0.1f || sData.inReverse) && sControls.brake) {
		sData.mVehicle->mCommandState.brakes[0] = sControls.throttle; // Brake becomes throttle
		sData.mVehicle->mCommandState.throttle = sControls.brake; // Throttle becomes brake
		sData.mVehicle->mEngineDriveState.gearboxState.currentGear = sData.neutralGear - 1; // Shifts to reverse
		sData.inReverse = true;
	}
	// Brake must be fully released
	else if (sControls.throttle && !sControls.brake) {
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

	if (useHealth && sData.health > 1)
		sData.health -= sData.boostUseRate * dt * 0.7f; // slower usage when using health
	else if (sData.boost > 0)
		sData.boost -= sData.boostUseRate * dt;

	// don't kill yourself from boosting
	if (sData.health < 1)
		sData.health = 1;

	// can't have negative boost
	if (sData.boost < 0)
		sData.boost = 0;
	
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
}

void SparkSys::regenBoost(SparkData& sData, double dt) {
	// Regenerate boost based on how 'hard' the drift is.
	// Traveling parallel to your lateral direction (90 deg from the direction you're 
	// facing) grants the maximum boost regeneration rate.
	if (sData.speed < sData.minDriftSpeed)
		return;

	const PxVec3 linVel = sData.rBody->getLinearVelocity();
	const PxVec3 lateral = sData.rBody->getGlobalPose().q.getBasisVector0(); // already normalized
	const float lateralSpeed = PxAbs(linVel.dot(lateral)); // don't want it to be negative

	// this will be the drift angle at which you have increased boost regeneration rate.
	// can calculate specified value as arccos(1 / threshold) = drift angle threshold.
	float threshold = 1.4f; // 1.15 ~= 29.6 deg, 1.5 ~= 48.2 deg, 2 = 60 deg

	// calculating the factor like this is much better for performance and gives similar results.
	// recall A.dot(B) = ||A|| ||B|| cosTheta   -->   cosTheta = A.dot(B) / (||A|| ||B||).
	// if cosTheta = 1 lateralSpeed is perfectly aligned with travel speed.
	// ensure you're traveling fast enough so to not divide by zero.
	float cosTheta = threshold * lateralSpeed / sData.speed; 
	sData.boost += sData.boostRegenRate * dt * cosTheta * cosTheta;
}

void SparkSys::applyShimmy(SparkData& sData, bool moveRight) {
	const PxVec3 lateralVector = sData.rBody->getGlobalPose().q.getBasisVector0();
	int flip = moveRight ? -1 : 1;
	
	sData.rBody->addForce(lateralVector * sData.shimmyForce * flip, PxForceMode::eVELOCITY_CHANGE);
	sData.shimmyTimer = sData.shimmyCooldown;

	float iFramesInSecs = sData.shimmyCooldown - sData.shimmyInvincible;
	angularResistance(sData, PX_MAX_REAL, iFramesInSecs);

}

void SparkSys::shimmy(SparkData& sData, SparkControls& sControls, double dt) {
	if (sData.shimmyTimer > 0) {
		sData.shimmyTimer -= dt;
		return;
	}

	if (sControls.shimmyL) {
		//dbug::log("GAME", 0, "slide to the left");
		applyShimmy(sData, false);
	}

	if (sControls.shimmyR) {
		//dbug::log("GAME", 0, "slide to the right");
		applyShimmy(sData, true);
	}
}

// HANDLING
void SparkSys::changeWheelParams(SparkData& sData, PxReal friction, PxReal latFriction, PxReal maxSteerAngle) {
	for (int i = 0; i < 4; i++) {
		sData.mVehicle->mBaseParams.tireForceParams[i].frictionVsSlip[2][1] = friction;
		sData.mVehicle->mBaseParams.tireForceParams[i].latStiffY = latFriction;
	}
	sData.mVehicle->mBaseParams.steerResponseParams.maxResponse = maxSteerAngle;
}

void SparkSys::driftStabilizer(SparkData& sData, SparkControls& sControls) {	
	const PxVec3 linVel = sData.rBody->getLinearVelocity();
	const PxVec3 forward = sData.rBody->getGlobalPose().q.getBasisVector2(); // already normalized
	const PxVec3 lateral = sData.rBody->getGlobalPose().q.getBasisVector0(); // already normalized
	const float lateralSpeed = linVel.dot(lateral);
	const float rollVel = sData.rBody->getAngularVelocity().x;
	const float yawVel = sData.rBody->getAngularVelocity().y;
	
	int ccw = PxSign(yawVel); // rotating ccw = 1 and cw = -1
	int countersteering = ccw * sControls.steering; // negative = steering out of corner, positive = steering into corner

	float driftCurve = 2.1f * sControls.steering;
	float angleCurve = 1.4f * sControls.steering;
	
	float cosTheta = lateralSpeed / sData.speed;
	
	PxVec3 totalForce(PxIdentity);
	PxVec3 totalTorque(PxIdentity);

	// slow down when hitting a hard drift, ~60 deg
	if (sData.speed > 40 && PxAbs(cosTheta) > 0.5f) {
		float speedDamp = 0.85f;
		float mass = sData.rBody->getMass();
		totalForce += (-linVel) * speedDamp * cosTheta * cosTheta * mass;
	}

	// Opposite forces automatically apply due to opposite sign when counter-steering
	PxVec3 driftDir = (forward + lateral * driftCurve);
	driftDir.y = 0.f;
	driftDir.normalize();
	float forceStrength = countersteering > 0 ? 1600.f : 400.f;
	totalForce += driftDir * forceStrength;
	
	float torqueStrength = countersteering > 0 ? -50.f : 600.f;
	totalTorque += PxVec3(0.f, 1.f, 0.f) * angleCurve * torqueStrength;
	
	float gripStrength = 240.f;
	totalForce += lateral * gripStrength * countersteering * cosTheta;
	

	float downforce = sData.speed * sData.speed * 0.2;
	totalForce += PxVec3(0.f, -downforce, 0.f);

	sData.rBody->addForce(totalForce);
	sData.rBody->addTorque(totalTorque);

	// this maks it really to hard to flip the car while cornering
	const float rollLimit = 0.1f;
	if (PxAbs(rollVel) > rollLimit) {
		PxVec3 angVel(PxClamp(rollVel, -rollLimit, rollLimit), yawVel, 0.f);
		sData.rBody->is<PxRigidDynamic>()->setAngularVelocity(angVel);
	}
}

void SparkSys::yawStabilizer(SparkData& sData) {
	const float yawVel = sData.rBody->getAngularVelocity().y;
	const float yawDamping = sData.speed * 0.15f;
	PxVec3 yawCorrection(0.f, -yawVel * yawDamping, 0.f);

	sData.rBody->addTorque(yawCorrection, PxForceMode::eACCELERATION);
}

void SparkSys::sparkHandling(SparkData& sData, SparkControls& sControls) {
	if (sControls.driftMode) {
		if (!sData.inDrift)
			changeWheelParams(sData, 11.4f, 54600, PxDegToRad(30));

		sData.inDrift = true;
		driftStabilizer(sData, sControls); // Helps control oversteer
	}
	else {
		// Reset friction params to original values from JSON
		if (sData.inDrift)
			changeWheelParams(sData, 3.8f, 145600, PxDegToRad(45));

		sData.inDrift = false;
		yawStabilizer(sData); // Helps prevent oversteer
	}
}

// RESPAWN
void SparkSys::sparkValuesReset(SparkData& sData) {
	sData.health = sData.maxHealth;
	sData.boost = sData.maxBoost;
	sData.shimmyTimer = 0;
	sData.deathTimer = sData.deathCooldown;
	sData.offGroundTimer = sData.offGroundLimit;
	
	sData.inReverse = false;
	sData.isBoosting = false;
	sData.isDead = false;

	dbug::log("GAME", 0, "reset spark");
}

void SparkSys::respawn(SparkData& sData, SparkControls& sControls, double dt) {
	if (sData.respawnTimer > 0) {
		sData.respawnTimer -= dt;
		return;
	}
	
	if (sData.deathTimer <= 0) {
		sparkValuesReset(sData);
		sControls.reset = true;
	}

	// TODO: make it so that after offGroundTimer is up, display a message indicating
	// you can use Y button (Xbox) to reset.
	if (sData.offGroundTimer <= 0 && !sData.isDead)
		sControls.reset = true;

	// Respawn logic in RespawnSystem::update
	if (sControls.reset)
		sData.respawnTimer = sData.respawnCooldown;
}