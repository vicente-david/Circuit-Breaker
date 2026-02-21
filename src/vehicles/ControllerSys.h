#pragma once

#include "GameState.h"
#include "ecs/System.h"

// this system takes the direct keyboard/controller inputs from the
// gamestate, and turns them into the commands to control the player car.
// this is pretty basic, but you it can be copied to use AI instead of player
// input, and the ecs should just make everything work
class ControllerSys : public System {
  public:
	void update(GameState &game);

	static std::shared_ptr<ControllerSys>
	registerSystem(std::shared_ptr<Coordinator> &coord);
};
