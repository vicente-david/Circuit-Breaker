#include "physics/Callbacks.h"
#include "PxActor.h"
#include "physics/CollisionData.h"

void PhysXCallbacks ::onContact(const PxContactPairHeader &pairHeader,
								const PxContactPair *pairs, PxU32 nbPairs) {

	// get collision data from each colliding actor
	CollisionData *d1 = (CollisionData *)pairHeader.actors[0]->userData;
	CollisionData *d2 = (CollisionData *)pairHeader.actors[1]->userData;

	dbug::log("PHYS", -1,
			  "Physics objects touched! If this crashes, a physics object "
			  "is likely missing its CollisionData");
	dbug::log("PHYS", 0, "collision: [1] typ:%d id:%d [2] typ:%d id:%d ",
			  d1->type, d1->entity, d2->type, d2->entity);

	// do things
	// theres probably a more straight forward way of doing this than like 10
	// layers of indirection but idk what it is
	if (d1->type == SPARK && d2->type == GROUND) {
		sparkWallCol.push_back(d1->entity);
	} else if (d1->type == GROUND && d2->type == SPARK) {
		sparkWallCol.push_back(d2->entity);
	} else if (d1->type == SPARK && d2->type == SPARK) {
		sparkSparkCol.push_back({d1->entity, d2->entity});
	}
}

void PhysXCallbacks::onTrigger(physx::PxTriggerPair *pairs,
							   physx::PxU32 count) {
	CollisionData *d1 = (CollisionData *)pairs[0].triggerActor->userData;
	CollisionData *d2 = (CollisionData *)pairs[0].otherActor->userData;
	dbug::log("PHYS", -1,
			  "Trigger touched! If this crashes, a physics object is "
			  "likely missing it's CollisionData");

	dbug::log("PHYS", 0, "trigger: [1] typ:%d id:%d [2] typ:%d id:%d ",
			  d1->type, d1->entity, d2->type, d2->entity);

	if (d1->type == SPARK && d2->type == FINISH_LINE) {
		dbug::log("GAME", 0, "finish!");
		sparkFinishCol.push_back(d1->entity);
	} else if (d1->type == FINISH_LINE && d2->type == SPARK) {
		dbug::log("GAME", 0, "finish!");
		sparkFinishCol.push_back(d2->entity);
	}
}
void PhysXCallbacks::resetLists() {
	sparkFinishCol.clear();
	sparkWallCol.clear();
	sparkSparkCol.clear();
}
