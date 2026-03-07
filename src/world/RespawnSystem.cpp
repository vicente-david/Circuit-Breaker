#include "RespawnSystem.h"
#include <glm/glm.hpp>
#include "debugUtils/Logger.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"

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

		if (eTransform.pos.y > yDeadzone) // entity hasn't fallen below the deadzone, skip
			continue;

		// entity fell below yDeadzone, respawn at last checkpoint
		dbug::log("GAME", 0, "Respawning entity %d", entity);

		// compute the respawn position: last checkpoint + deltaY above it
		glm::vec3 respawnPos = lapProg.lastCheckpointPos;
		respawnPos.y += deltaY;

		// teleport the PhysX rigid body to the respawn position
		auto& rBody = game.coordinator->getComponent<physx::PxRigidBody*>(entity);
		physx::PxTransform respawnPose(
			physx::PxVec3(respawnPos.x, respawnPos.y, respawnPos.z),
			physx::PxQuat(physx::PxIdentity) // reset rotation to upright
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
	}
}
