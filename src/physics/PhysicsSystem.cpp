#include "PhysicsSystem.h"
#include "GameState.h"
#include "PxRigidBody.h"
#include "ecs/Component.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include <memory>

// convinence function to create/register the physics system in the ecs
std::shared_ptr<PhysicsSystem>
PhysicsSystem::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<PhysicsSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<physx::PxRigidBody *>());
	sig.set(coord->getComponentType<Transform>());
	coord->setSystemSignature<PhysicsSystem>(sig);

	return system;
}

// this is called every frame
void PhysicsSystem::updatePhysics(double dt, GameState &game) {
	game.physics->updatePhysics(dt);
	updateTransforms(game);
}

void PhysicsSystem::updateTransforms(GameState &state) {
	// update the transform to match the state of the physics simulation
	for (auto const &entity : entities) {
		auto &body =
			state.coordinator->getComponent<physx::PxRigidBody *>(entity);
		auto &transform = state.coordinator->getComponent<Transform>(entity);

		physx::PxVec3 p = body->getGlobalPose().p;
		physx::PxQuat q = body->getGlobalPose().q;
		physx::PxVec3 B3 = q.getBasisVector2();
		transform.pos = glm::vec3(p.x, p.y, p.z);
		transform.rot = glm::quat(q.x, q.y, q.z, q.w);
		transform.forwardD = glm::vec3(B3.x, B3.y, B3.z);
		// dbug::log("PHYS", -1, "Entity %d at [%f, %f, %f]", entity, p.x, p.y,
		// p.z);
	}
}
