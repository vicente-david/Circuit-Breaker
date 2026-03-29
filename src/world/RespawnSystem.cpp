#include "RespawnSystem.h"
#include <glm/glm.hpp>
#include "debugUtils/Logger.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"
#include "vehicles/SparkComponents.h"

std::shared_ptr<RespawnSystem> RespawnSystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	auto system = coord->registerSystem<RespawnSystem>();

	Signature sig;
	sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<Respawnable>());
	sig.set(coord->getComponentType<LapCounter>());
	sig.set(coord->getComponentType<physx::PxRigidBody*>());

	coord->setSystemSignature<RespawnSystem>(sig);

	return system;
}

void RespawnSystem::update(GameState& game) {
	// iterate through all entities in the game that are respawnable
	for (auto& entity : entities) {
		Transform& eTransform = game.coordinator->getComponent<Transform>(entity);
		LapCounter& lapProg = game.coordinator->getComponent<LapCounter>(entity);
		SparkControls& sControls = game.coordinator->getComponent<SparkControls>(entity);

		if (eTransform.pos.y > yDeadzone && !sControls.reset) // entity hasn't fallen below the deadzone, skip
			continue;

		// entity fell below yDeadzone, respawn at last checkpoint
		dbug::log("RESPAWN", 0, "Respawning entity %d", entity);

		// compute the respawn position: last checkpoint + deltaY above it
		glm::vec3 respawnPos = lapProg.lastCheckpointPos;
		respawnPos.y += deltaY;

		// if the entity has an ai controller component, reset its understanding of where it is on the track
		if (game.coordinator->hasComponent<AIController>(entity)) {
			AIController& ai = game.coordinator->getComponent<AIController>(entity);
			SparkData& spark = game.coordinator->getComponent<SparkData>(entity);
			
			ai.respawnRecoverTimer = 20; // sets the lookahead value low for this number of iterations after respawning (helps with 'resetting' steering)
			ai.lookAheadSteps = 1;
			ai.currentPosIdx = lapProg.lastCheckpointIdx;
			ai.lastPosIdx = ai.currentPosIdx - 1;

			// if the position at the last checkpoint index on the route the ai is currently on is NOT the same as the checkpoint respawned at,
			// override what path the ai thinks its on
			if (ai.route.at(lapProg.lastCheckpointIdx) != lapProg.lastCheckpointPos) {
				dbug::log("AIPATH", 1, "RespawnSys detects wrong path for %s.", spark.mVehicleName.c_str());
				// force the desired path to switch
				ai.routeID = static_cast<PathID>((static_cast<int>(ai.routeID) + 1) % 2);
				int i = static_cast<int>(ai.routeID);
				ai.route = paths[i].curvePoints;
				ai.angles = paths[i].curvatures;
			}
		}

		// compute the respawn rotation so the vehicle faces along the track
		// the vehicle's forward axis is +Z, so we need the angle from +Z to lastCheckpointDir
		glm::vec3 trackDir = lapProg.lastCheckpointDir; // already XZ-normalized by LapSystem
		float yawAngle = glm::atan(trackDir.x, trackDir.z); // angle from +Z toward +X
		// PxQuat(angle, axis) creates a rotation of 'angle' radians around 'axis'
		physx::PxQuat respawnRot(yawAngle, physx::PxVec3(0.0f, 1.0f, 0.0f)); // rotate around Y axis

		// teleport the PhysX rigid body to the respawn position
		auto& rBody = game.coordinator->getComponent<physx::PxRigidBody*>(entity);
		physx::PxTransform respawnPose(
			physx::PxVec3(respawnPos.x, respawnPos.y, respawnPos.z),
			respawnRot
		);
		rBody->setGlobalPose(respawnPose);

		// zero out velocity so the entity doesn't keep falling/moving
		physx::PxRigidDynamic* dynamicBody = rBody->is<physx::PxRigidDynamic>();
		if (dynamicBody) {
			dynamicBody->setLinearVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
			dynamicBody->setAngularVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
		}

		// PhysicsSystem will sync the new PhysX position into Transform on the next frame,
		// but update it now too so the camera/rendering sees it immediately this frame
		eTransform.pos = respawnPos;
		sControls.reset = false; // so AI doesn't get stuck in a loop
	}
}

void RespawnSystem::setPaths(std::vector<TrackCurve>& paths) {
	this->paths = paths;
}
