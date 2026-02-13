#include "Vehicle.h"
#include "InputSystem.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

Vehicle::Vehicle(PhysicsSystem& physicsSystem)
	: mPhysics(physicsSystem)
{}

bool Vehicle::init()
{
	//Load the params from json or set directly.
	mVehicleDataPath = "assets/vehicledata";
	readBaseParamsFromJsonFile(mVehicleDataPath, "Base.json", mVehicle.mBaseParams);
	readEngineDrivetrainParamsFromJsonFile(mVehicleDataPath, "EngineDrive.json", mVehicle.mEngineDriveParams);
	setPhysXIntegrationParams(
		mVehicle.mBaseParams.axleDescription,
		mMaterialFrictions,
		mNbMaterialFrictions,
		mDefaultMaterialFriction,
		mVehicle.mPhysXParams
	);

	//Set the states to default.
	if (!mVehicle.initialize(
		*mPhysics.gPhysics,
		PxCookingParams(PxTolerancesScale()),
		*mPhysics.gMaterial,
		EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE
		))
	{
		return false;
	}

	//Apply a start pose to the physx actor and add it to the physx scene.
	PxTransform startPose(PxVec3(0.000000000f, -0.0500000119f, -50.59399998f), PxQuat(PxIdentity));
	mVehicle.setUpActor(*mPhysics.gScene, startPose, mVehicleName);
	// Create vehicle filter
	PxFilterData vehicleFilter(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
	
	// Set flags
	PxU32 shapes = mVehicle.mPhysXState.physxActor.rigidBody->getNbShapes();
	for (PxU32 i = 0; i < shapes; i++) {
		PxShape* shape = NULL;
		mVehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, i);

		shape->setSimulationFilterData(vehicleFilter); // Add filter data to shader

		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}

	//Set the vehicle in 1st gear.
	mVehicle.mEngineDriveState.gearboxState.currentGear = mVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	mVehicle.mEngineDriveState.gearboxState.targetGear = mVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	//Set the vehicle to use the automatic gearbox.
	mVehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

	//Set up the simulation context.
	//The snippet is set up with
	//a) z as the longitudinal axis
	//b) x as the lateral axis
	//c) y as the vertical axis.
	//d) metres  as the lengthscale.
	mVehicleSimContext.setToDefault();
	
	mVehicleSimContext.frame.lngAxis = PxVehicleAxes::ePosZ;
	mVehicleSimContext.frame.latAxis = PxVehicleAxes::ePosX;
	mVehicleSimContext.frame.vrtAxis = PxVehicleAxes::ePosY;
	mVehicleSimContext.scale.scale = 1.0f;

	mVehicleSimContext.gravity = mPhysics.gGravity;
	mVehicleSimContext.physxScene = mPhysics.gScene;
	mVehicleSimContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;

	return true;
}

void Vehicle::cleanup()
{
	mVehicle.destroy();
}

void Vehicle::step(double dt)
{
	// Command input = { 0.0f, 0.65f, 0.0f, mVehicle.mTransmissionCommandState.targetGear};
	// applyInput(input);

	// VEHICLE SCENE QUERIES
	//Forward integrate the vehicle by a single timestep.
	//Apply substepping at low forward speed to improve simulation fidelity.
	const PxVec3 linVel = mVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
	const PxVec3 forwardDir = mVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
	
	const PxReal speed = linVel.dot(forwardDir);
	const PxU8 nbSubsteps = (speed < 5.0f ? 3 : 1);
	
	mVehicle.mComponentSequence.setSubsteps(mVehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);
	mVehicle.step(dt, mVehicleSimContext);

	updateTransform();
}

void Vehicle::applyInput(Actions& actions, double dt)
{
	mVehicle.mCommandState.brakes[0] = actions.moveBackward;
	mVehicle.mCommandState.nbBrakes = 1;
	mVehicle.mCommandState.throttle = actions.moveForward;
	mVehicle.mCommandState.steer = actions.xRotation;
	
	if (actions.boost) {
		if (boostAmount > 0)
			Boost();
	}
	else if (boostAmount < boostMax) {
			boostAmount += boostRegenerate;
			if (boostAmount > boostMax)
				boostAmount = boostMax;
			std::cout << "Boost at: " << boostAmount << "%" << std::endl;
	}

	if (shimmyActiveTimer == 0) {
		if (actions.shimmyRight)
			Shimmy(true);

		if (actions.shimmyLeft)
			Shimmy(false);
	}
	else {
		shimmyActiveTimer -= dt;
		if (shimmyActiveTimer <= 0) {
			shimmyActiveTimer = 0;
			std::cout << "Shimmy ready!" << std::endl;
		}
	}
}

void Vehicle::changeEngineDriveParams(const char* vehicleDataFileName) 
{
	// Changes the parameters of the engine
	readEngineDrivetrainParamsFromJsonFile(mVehicleDataPath, vehicleDataFileName, mVehicle.mEngineDriveParams);
}

void Vehicle::updateTransform() {
	
	PxVec3 p = mVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxQuat q = mVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q;
	PxVec3 B3 = q.getBasisVector2();
	transform.pos = glm::vec3(p.x, p.y, p.z);
	transform.rot = glm::quat(q.x, q.y, q.z, q.w);
	transform.forwardD = glm::vec3(B3.x, B3.y, B3.z);
}

void Vehicle::Boost() {
	const PxVec3 forwardDir = mVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
	float boostStrength = 10.f;
	
	boostAmount -= 0.5f;

	mVehicle.mPhysXState.physxActor.rigidBody->addForce(forwardDir * boostStrength, PxForceMode::eACCELERATION);
	std::cout << "BOOST!" << std::endl;
}

void Vehicle::Shimmy(bool rightDir) {
	const PxVec3 latDir = mVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector0();
	int flip = (rightDir) ? -1 : 1;
	float shimmyForce = 24000.f;
	
	mVehicle.mPhysXState.physxActor.rigidBody->addForce(latDir * shimmyForce * flip, PxForceMode::eIMPULSE);
	std::cout << "Weeeeeeee!!" << std::endl;
	
	shimmyActiveTimer = shimmyCooldown; // 
}
