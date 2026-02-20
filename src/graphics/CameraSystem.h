#pragma once

#include "GameState.h"
#include "ecs/Coordinator.h"
#include "ecs/System.h"
#include "graphics/Camera.h"
#include "graphics/CameraComp.h"
#include <glm/fwd.hpp>
#include <memory>
#include <vector>


class CameraSystem : public System {
  public:
	const int MAX_CAMS = 1;
	// store pointers to all the cameras so rendering can use them
	std::vector<std::shared_ptr<CameraComp>> cameras;

	CameraSystem() {
		// create 1 camera by default
		cameras = std::vector<std::shared_ptr<CameraComp>>(1);
	}
	// helper to register the signature in the ECS
	static std::shared_ptr<CameraSystem>
	registerSystem(std::shared_ptr<Coordinator> &coord);

	void update(GameState &gamestate, float dt);
};
