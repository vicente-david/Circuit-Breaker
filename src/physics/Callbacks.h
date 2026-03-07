#pragma once

#include "PxSimulationEventCallback.h"
#include "ecs/EntityManager.h"
#include <utility>
#include <vector>

using namespace physx;

// this is probably way to abstracted, but i couldn't think of a better way to make this work
// you kinda need to jump through like 9999 hoops just to detect a collision, sorry 
class PhysXCallbacks : public PxSimulationEventCallback {

	public:
	std::vector<Entity> sparkFinishCol;
	std::vector<Entity> sparkWallCol;
	std::vector<std::pair<Entity, Entity>> sparkSparkCol;
	//... add arrays for the collisions you need to detect

	void resetLists();

	protected:
	void onContact(const PxContactPairHeader &pairHeader,
				   const PxContactPair *pairs, PxU32 nbPairs);

	void onConstraintBreak(physx::PxConstraintInfo *constraints,
						   physx::PxU32 count) {}
	void onWake(physx::PxActor **actors, physx::PxU32 count) {}
	void onSleep(physx::PxActor **actors, physx::PxU32 count) {}
	void onTrigger(physx::PxTriggerPair *pairs, physx::PxU32 count);
	void onAdvance(const physx::PxRigidBody *const *bodyBuffer,
				   const physx::PxTransform *poseBuffer,
				   const physx::PxU32 count) {}

};
