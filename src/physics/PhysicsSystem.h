#pragma once

#include "GameState.h"
#include <memory>

#include "ecs/Coordinator.h"

// this is the physics system in ECS. all it really does it tells the
// physics manager to update physx, then updats the transforms of all the
// objects
// this could probably be combined with the manager, but you end up with 
// circular dependencies, and I feel like this is a bit cleaner
class PhysicsSystem : public System {
  public:
	void updateTransforms(GameState &state);

	void updatePhysics(double dt, GameState &gameState);

	static std::shared_ptr<PhysicsSystem>
	registerSystem(std::shared_ptr<Coordinator> &coord);
};
