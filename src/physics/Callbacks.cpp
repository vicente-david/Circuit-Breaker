#include "physics/Callbacks.h"
#include "PxActor.h"
#include "PxFiltering.h"
#include "PxRigidBody.h"
#include "PxSimulationEventCallback.h"
#include "debugUtils/Logger.h"
#include "ecs/EntityManager.h"
#include "foundation/PxConstructor.h"
#include "foundation/PxSimpleTypes.h"
#include "foundation/PxVec3.h"
#include "physics/CollisionData.h"
#include <cstdio>
#include <set>
#include <geomutils/PxContactPoint.h>

void PhysXCallbacks ::onContact(const PxContactPairHeader &pairHeader,
								const PxContactPair *pairs, PxU32 nbPairs) {

	dbug::log("PHYS", -1,
			  "Physics objects touched! If this crashes, a physics object "
			  "is likely missing its CollisionData");
	// get collision data from each colliding actor
	CollisionData *d1 = (CollisionData *)pairHeader.actors[0]->userData;
	CollisionData *d2 = (CollisionData *)pairHeader.actors[1]->userData;

	dbug::log("PHYS", 0, "collision: [1] typ:%d id:%d [2] typ:%d id:%d ",
			  d1->type, d1->entity, d2->type, d2->entity);

	// spark/wall collisions
	if (d1->type == SPARK && d2->type == WALL) {
		auto vel = ((PxRigidBody *)pairHeader.actors[0])->getLinearVelocity();
		auto imp = getCollStrength(pairs, nbPairs, vel);
		sparkWallCol.push_back(SparkWallColData{ d1->entity, imp});

	} else if (d1->type == WALL && d2->type == SPARK) {
		auto vel = ((PxRigidBody *)pairHeader.actors[1])->getLinearVelocity();
		auto imp = getCollStrength(pairs, nbPairs, vel);
		sparkWallCol.push_back(SparkWallColData{ d1->entity, imp });

	} else if (d1->type == SPARK && d2->type == SPARK) {
		// get collision velocity
		auto vel = ((PxRigidBody *)pairHeader.actors[0])->getLinearVelocity() -
				   ((PxRigidBody *)pairHeader.actors[1])->getLinearVelocity();
		// get strength related to the angle of collision
		auto imp = getCollStrength(pairs, nbPairs, vel);
		// point of contact
		auto pt = (PxContact*)pairs->contactPoints;
		auto pp = (PxContactPoint*)pairs->contactPoints;
		PxVec3 contact = pt->contact;
		PxVec3 cNormal = pp->normal;
		
		// send data to spark system
		sparkSparkCol.push_back(SparkSparkColData{d1->entity, d2->entity, imp, contact, vel, cNormal});
		

	// death plane
	}else if (d1->type == SPARK && d2->type == KILL) {
		killSparks.push_back(d1->entity);
	}else if (d2->type == SPARK && d1->type == KILL) {
		killSparks.push_back(d2->entity);
	} 
}

void updateEntityCollider(std::set<Entity> &set, PxPairFlag::Enum status,
						  Entity e) {
	if (status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
		set.insert(e);
	} else if (status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
		set.erase(e);
	}
}
void PhysXCallbacks::onTrigger(physx::PxTriggerPair *pairs,
							   physx::PxU32 count) {

	dbug::log("PHYS", -1,
			  "Trigger touched! If this crashes, a physics object is "
			  "likely missing it's CollisionData");
	for (int pIdx = 0; pIdx < count; pIdx++) {
		CollisionData *trigData =
			(CollisionData *)pairs[pIdx].triggerActor->userData;
		CollisionData *otherData =
			(CollisionData *)pairs[pIdx].otherActor->userData;
		auto status = pairs[pIdx].status;
		dbug::log("PHYS", 0,
				  "tIdx %d: status: %d [t] typ:%d id:%d [o] typ:%d id:%d ",
				  pIdx, pairs[pIdx].status, trigData->type, trigData->entity,
				  otherData->type, otherData->entity);

		// ground trigger box for the spark
		if (trigData->type == SPARK) {
			if (otherData->type == GROUND) {
				updateEntityCollider(groundedSparks, status, trigData->entity);
			}
			if (otherData->type == HEAL) {
				updateEntityCollider(groundedSparks, status, trigData->entity);
				updateEntityCollider(healingSparks, status, trigData->entity);
			}
		}
	}
}
// PxVec3 PhysXCallbacks::getCollStrength(const PxContactPair *pairs,
// 									   PxU32 nbPairs) {
// 	// get collision data for every shape that intersects
// 	// impuse doesnt want to work :(
// 	float total;
// 	printf("pairs:%d\n", nbPairs);
// 	for (int pairIdx = 0; pairIdx < nbPairs; pairIdx++) {
// 		auto &cp = pairs[pairIdx];
// 		PxContactPairPoint contacts[16];
// 		int contCount = cp.extractContacts(contacts, cp.contactCount);
//
// 		printf("points:%d or %d\n", cp.contactCount, contCount);
//
// 		for (int pointIdx = 0; pointIdx < contCount; pointIdx++) {
// 			auto imp = contacts[pointIdx].normal;
// 			total += contacts->impulse[pointIdx];
//
// 			dbug::log("PHYS", 0, "pair:%d point:%d impulse:%f", pairIdx,
// 					  pointIdx, contacts->impulse[pointIdx]);
// 		}
// 	}
// 	return PxVec3(total, 0, 0);
// 	// return total;
// }

// TODO: use the impulse here instead of just linear velocity.
// stuff above should work, but the numbers don't seem right to me so idk
float PhysXCallbacks::getCollStrength(const PxContactPair* pairs, PxU32 nbPairs, PxVec3 velocity) {
	float totalImpulse = 0.f;
	for (PxU32 pairIdx = 0; pairIdx < nbPairs; pairIdx++) {
		const PxContactPair& cp = pairs[pairIdx];
		PxContactPairPoint contacts[16];
		if (cp.contactCount > 16)
			dbug::log("PHYS", 1, "WARNING: contact overflow (%d)", cp.contactCount);

		PxU32 count = cp.extractContacts(contacts, 16);

		for (PxU32 i = 0; i < count; i++) {
			const PxVec3& norm = contacts[i].normal;
			const PxVec3& impulse = contacts[i].impulse;
			float normImpulse = impulse.dot(norm);

			if (normImpulse > 0.f)
				totalImpulse += normImpulse;
		}
	}
	float maxImpulse = velocity.magnitudeSquared() * 100;
	dbug::log("PHYS", 0, "Impulse: %f, MaxImpulse %f", totalImpulse, maxImpulse);
	return PxMin(totalImpulse / maxImpulse, 10.f);
}

void PhysXCallbacks::resetLists() {
	sparkFinishCol.clear();
	sparkWallCol.clear();
	sparkSparkCol.clear();
	killSparks.clear();
}

// Special callbacks that allow you to modify collision point data for a specific type of collision
void ModifiedCallbacks::onContactModify(PxContactModifyPair* const pairs, PxU32 nbPairs) {
    // get collision data from each colliding actor
    CollisionData* d1 = (CollisionData*)pairs->actor[0]->userData;
    CollisionData* d2 = (CollisionData*)pairs->actor[1]->userData;

    if (d1->type == SPARK && d2->type == WALL) {
        PxContactSet& contact = pairs->contacts;
        contact.setInvMassScale0(0.1f); // Set mass of spark for this collision (any value over 1.f here makes the mass behave lighter and <1.f heavier)
		contact.setInvInertiaScale0(0.f); // Infinite inertia: prevents spinning out from hitting wings on the walls
		
		for (int i = 0; i < contact.size(); i++) {
			PxVec3 pushDir = contact.getNormal(i); // getNormal is from contact 1 to 2
			pushDir.x *= 7.f;
			pushDir.z *= 7.f;
			contact.setTargetVelocity(i, pushDir);
			//contact.setMaxImpulse(i, 5.f);
		}
		
    }
    else if (d1->type == WALL && d2->type == SPARK) {
        PxContactSet& contact = pairs->contacts;
        contact.setInvMassScale1(0.1f);
		contact.setInvInertiaScale1(0.f);
		
		for (int i = 0; i < contact.size(); i++) {
			PxVec3 pushDir = -contact.getNormal(i); // getNormal is from contact 1 to 2
			pushDir.x *= 7.f;
			pushDir.z *= 7.f;
			contact.setTargetVelocity(i, pushDir);
			//contact.setMaxImpulse(i, 5.f);
		}
    }

}
