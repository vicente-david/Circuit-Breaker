#pragma once

#include "GameState.h"
#include "ecs/Coordinator.h"
#include "ecs/System.h"
#include "graphics/Camera.h"
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

// TODO: combine this with the other camera class
struct CameraComp {

	int camNumber = 0;
	glm::vec3 position;
	float posEasing = 0.2;
	// add other things as needed for cool camera movement
};

class CameraSystem : public System {
  public:
	const int MAX_CAMS = 1;
	std::vector<Camera> cameras;

	CameraSystem() {
		// create 1 camera by default
		cameras = std::vector<Camera>(1);
	}
	// helper to register the signature in the ECS
	static std::shared_ptr<CameraSystem>
	registerSystem(std::shared_ptr<Coordinator> &coord);

	void update(GameState &gamestate, float dt);
};
