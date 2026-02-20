#pragma once

#include "GameState.h"
#include "ecs/System.h"

class ControllerSys : public System{

	public:

	void update(GameState& game);

	static std::shared_ptr<ControllerSys>
	registerSystem(std::shared_ptr<Coordinator> &coord);
};
