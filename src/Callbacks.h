#pragma once

#include "GameState.h"
#include "PxPhysicsAPI.h"
#include "PxSimulationEventCallback.h"
#include "debugUtils/Logger.h"
#include "physics/CollisionData.h"
#include <iostream>

using namespace physx;

class ContactReportCallback : public PxSimulationEventCallback {
	// GameState& gamestate;
	void onContact(const PxContactPairHeader &pairHeader,
				   const PxContactPair *pairs, PxU32 nbPairs) {


		CollisionData *d1 = (CollisionData *)pairHeader.actors[0]->userData;
		CollisionData *d2 = (CollisionData *)pairHeader.actors[1]->userData;

		dbug::log("PHYS", 0,
				  "Physics objects touched! If this crashes, a physics object is likely "
				  "missing a CollisionData reference in its userData (ask "
				  "matthew about this if you're confused)");
		dbug::log("PHYS", 0, "collision: [1] typ:%d id:%d [2] typ:%d id:%d ",
				  d1->type, d1->entity, d2->type, d2->entity);
	}
	void onConstraintBreak(physx::PxConstraintInfo *constraints,
						   physx::PxU32 count) {}
	void onWake(physx::PxActor **actors, physx::PxU32 count) {}
	void onSleep(physx::PxActor **actors, physx::PxU32 count) {}
	void onTrigger(physx::PxTriggerPair *pairs, physx::PxU32 count) {
		CollisionData *d1 = (CollisionData *)pairs[0].triggerActor->userData;
		CollisionData *d2 = (CollisionData *)pairs[0].otherActor->userData;
		dbug::log("PHYS", 0,
				  "Trigger touched! If this crashes, a physics object is likely "
				  "missing a CollisionData reference in its userData (ask "
				  "matthew about this if you're confused)");

		dbug::log("PHYS", 0, "trigger: [1] typ:%d id:%d [2] typ:%d id:%d ",
		d1->type, d1->entity, d2->type, d2->entity);
	}
	void onAdvance(const physx::PxRigidBody *const *bodyBuffer,
				   const physx::PxTransform *poseBuffer,
				   const physx::PxU32 count) {}
};
