//#include "Vehicle.h"
//
//using namespace physx;
//using namespace physx::vehicle2;
//using namespace snippetvehicle;
//
//Vehicle::Vehicle(PhysicsSystem& physicsSystem)
//	: mPhysics(physicsSystem)
//{}
//
//bool Vehicle::init()
//{
//	const char* vehicleDataPath = "snippets/media/vehicledata";
//
//	readBaseParamsFromJsonFile(vehicleDataPath, "Base.json", mVehicle.mBaseParams);
//	readEngineDrivetrainParamsFromJsonFile(vehicleDataPath, "EngineDrive.json", mVehicle.mEngineDriveParams);
//	setPhysXIntegrationParams(
//		mVehicle.mBaseParams.axleDescription,
//		mMaterialFrictions,
//		mNbMaterialFrictions,
//		mDefaultMaterialFriction,
//		mVehicle.mPhysXParams
//	);
//
//	if (!mVehicle.initialize(
//		*mPhysics.gPhysics,
//		PxCookingParams(PxTolerancesScale()),
//		*mPhysics.gMaterial,
//		EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE
//		))
//	{
//		return false;
//	}
//
//	PxTransform pose(PxVec3(0.f, 1.f, 0.f), PxQuat(PxIdentity));
//	mVehicle.setUpActor(*mPhysics.gScene, pose, mVehicleName);
//
//	mVehicle.mEngineDriveState.gearboxState.currentGear = mVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
//	mVehicle.mEngineDriveState.gearboxState.targetGear = mVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
//	mVehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;
//
//	mVehicleSimContext.setToDefault();
//	mVehicleSimContext.gravity = mPhysics.gGravity;
//
//	mVehicleSimContext.physxScene = mPhysics.gScene;
//	mVehicleSimContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;
//
//	return true;
//}
//
//
//void Vehicle::cleanup()
//{
//	mVehicle.destroy();
//}
//
//void Vehicle::step(double dt)
//{
//	const PxVec3 linVel = mVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
//	const PxVec3 forwardDir = mVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
//	
//	const PxReal speed = linVel.dot(forwardDir);
//	const PxU8 nbSubsteps = (speed < 5.0f ? 3 : 1);
//	
//	mVehicle.mComponentSequence.setSubsteps(mVehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);
//	mVehicle.step(dt, mVehicleSimContext);
//}
//
//void Vehicle::setInput(const Command& cmd)
//{
//	auto& mCmd = mVehicle.mCommandState;
//	mCmd.brakes[0] = cmd.brake;
//	mCmd.nbBrakes = 1;
//	mCmd.throttle = cmd.throttle;
//	mCmd.steer = cmd.steer;
//	mVehicle.mTransmissionCommandState.targetGear = cmd.gear;
//}