
#include "CameraSystem.h"
#include "GameState.h"
#include "debugUtils/Logger.h"
#include "ecs/Component.h"
#include "ecs/Coordinator.h"
#include "graphics/Camera.h"
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

		// smoothly interpolate distance
		//  camData.position = (camData.position+transform.pos) *
		//  camData.posEasing;
		camData.position = transform.pos;

		// update camera for this object
		if (camData.camNumber >= MAX_CAMS) {
			dbug::log(
				"REND", 2,
				"Cam number %d is larger than the maximum number of cameras!",
				camData.camNumber);
		}
		// add cameras if there isn't enough
		while (cameras.size() < camData.camNumber + 1) {
			cameras.push_back(Camera());
		}

		dbug::log("REND", -1, "cam pos: [%f, %f, %f]", camData.position.x,
				  camData.position.y, camData.position.z);
		auto &cam = cameras[camData.camNumber];
		cam.updateCamera(camData.position, transform.forwardD,
						 game.inputActions.camXRot, dt,
						 game.inputActions.cameraReset);


		// update the audio listner's frame for 3d audio
		//TODO: set this to real camera velocty
		game.audio->updateListenerVel(0, 0, 0);
		game.audio->updateListenerFrame(cam.GetViewMatrix());
	}
}
