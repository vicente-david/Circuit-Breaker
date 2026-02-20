
#include "CameraSystem.h"
#include "GameState.h"
#include "debugUtils/Logger.h"
#include "ecs/Component.h"
#include "ecs/Coordinator.h"
#include "graphics/Camera.h"
#include "graphics/CameraComp.h"
#include <glm/ext/quaternion_common.hpp>
#include <glm/fwd.hpp>
#include <glm/vector_relational.hpp>
#include <memory>

std::shared_ptr<CameraSystem>
CameraSystem::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<CameraSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	// sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<CameraComp>());
	coord->setSystemSignature<CameraSystem>(sig);

	return system;
}

void CameraSystem::update(GameState &game, float dt) {

	for (auto &entity : entities) {
		auto &camData = game.coordinator->getComponent<CameraComp>(entity);
		auto &transform = game.coordinator->getComponent<Transform>(entity);

		// save the starting position so we can calculate velocity later
		auto startLocation = camData.position;

		// move camera to where it's proper position/orientation

		// get the position the camera wants to be based on car position
		camData.carPosition = transform.pos;
		auto targetPos = camData.targetPosition(transform.rot);

		// interpolate between target and current position
		// if we are in an invalid position, just use the target position
		if (glm::all(glm::isnan(camData.position))) {
			camData.position = targetPos;
		} else {
			camData.position = targetPos * camData.posEasing +
							   camData.position * (1 - camData.posEasing);
		}
		camData.up = camData.carUp;

		// update camera for this object
		if (camData.camNumber >= MAX_CAMS) {
			dbug::log(
				"REND", 2,
				"Cam number %d is larger than the maximum number of cameras!",
				camData.camNumber);
		}

		glm::vec3 carOffset = transform.forwardD * camData.lookAtdistance;
		camData.carPosition += carOffset;
		// add cameras if there isn't enough
		while (cameras.size() < camData.camNumber + 1) {
			cameras.push_back(std::make_shared<CameraComp>());
		}
		cameras[camData.camNumber] = std::make_shared<CameraComp>(camData);

		dbug::log("REND", -1, "cam pos: [%f, %f, %f]", camData.position.x,
				  camData.position.y, camData.position.z);

		auto camVel = (camData.carPosition - startLocation)*(1.0f/dt);
		
		// update the audio listner's frame and velocity for 3d audio
		game.audio->updateListenerVel(camVel.x,camVel.y, camVel.z);
		game.audio->updateListenerFrame(camData.GetViewMatrix());
	}
}
