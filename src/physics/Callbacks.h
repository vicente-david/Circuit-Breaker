#pragma once

#include "PxSimulationEventCallback.h"
#include "ecs/EntityManager.h"
#include "foundation/PxVec3.h"
#include <set>
#include <utility>
#include <vector>

using namespace physx;

// aaaaaah
struct SparkWallColData {
	Entity sparkId;
	float magnitude;
};
struct SparkSparkColData {
	Entity spark1Id;
	Entity spark2Id;
	float magnitude;
};

// this is probably way to abstracted, but i couldn't think of a better way to
// make this work. you kinda need to jump through a few hoops just to detect a
// collision, sorry
class PhysXCallbacks : public PxSimulationEventCallback {

  public:
	std::set<Entity> groundedSparks;
	std::set<Entity> healingSparks;
	std::vector<Entity> sparkFinishCol;
	std::vector<SparkWallColData> sparkWallCol;
	std::vector<SparkSparkColData> sparkSparkCol;
	//... add arrays with the data for whatever collision you need to detect

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

  private:
	// PxVec3 getCollStrength(const PxContactPair *pairs, PxU32 nbPairs);
	float getCollStrength(const PxContactPair *pairs, PxU32 nbPairs,
						  PxVec3 velocity);
};
