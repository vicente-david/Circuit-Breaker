#pragma once

#include "GameState.h"
#include "ecs/Coordinator.h"
#include "ecs/System.h"
#include <memory>
class AudioSystem : public System {
  public:
	static std::shared_ptr<AudioSystem>
	registerSystem(std::shared_ptr<Coordinator> &coord);

	void updateSounds(GameState &gameState);
};
