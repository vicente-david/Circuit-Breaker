#pragma once
#include "../GameState.h"
#include "ecs/System.h"
#include "ecs/Coordinator.h"
#include "ecs/Component.h"
#include "LapSystem.h"
#include "ai/AIControllerSys.h"

struct Respawnable {
	// empty struct to act as a flag that the entity attached to this system can be respawned
};

class RespawnSystem : public System {
public:
	static std::shared_ptr<RespawnSystem> registerSystem(std::shared_ptr<Coordinator>& coord);
	void update(GameState& game); // does the respawn logic, probably safe to check every frame


private:
	float yDeadzone = -20.0f; // Y value which dictates when we respawn
	float deltaY = 2.0f; // "how much further above should we respawn over the last checkpoint?"
};