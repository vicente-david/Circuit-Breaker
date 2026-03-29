
#include "CameraSystem.h"
#include "GameState.h"
#include "PxRigidBody.h"
#include "debugUtils/Logger.h"
#include "graphics/CameraComp.h"
#include <glm/ext/quaternion_common.hpp>

std::shared_ptr<CameraSystem>
CameraSystem::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<CameraSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<PxRigidBody *>());
	sig.set(coord->getComponentType<CameraComp>());
	coord->setSystemSignature<CameraSystem>(sig);

	return system;
}

void CameraSystem::update(GameState &game, float dt) {

	for (auto &entity : entities) {
		auto &camData = game.coordinator->getComponent<CameraComp>(entity);
		auto &rBody = game.coordinator->getComponent<PxRigidBody *>(entity);
		auto &transform = game.coordinator->getComponent<Transform>(entity);

		// save the starting position so we can calculate velocity later
		auto startLocation = camData.position;

		// move camera to where it's proper position/orientation

		// camData.yaw = (1 - camData.angleEasing) * camData.yaw +
		// 			  camData.angleEasing * (game.inputActions.camXRot * 90);
		// camData.pitch = (1 - camData.angleEasing) * camData.pitch +
		// 			  camData.angleEasing * (30+game.inputActions.camXRot * 30);

		float targetYaw = game.inputActions.camXRot * 90;
		camData.yaw = (1 - camData.yawEasing) * camData.yaw +
					  camData.yawEasing * targetYaw;

		// if (std::abs(camData.yaw) < std::abs(targetYaw)) {
		// 	camData.yaw = targetYaw;
		// } else {
		// 	if (camData.yaw > 0) {
		// 		camData.yaw =
		// 			std::max(0.f, camData.yaw - dt * camData.angleSpeed);
		// 	}
		// 	if (camData.yaw < 0) {
		// 		camData.yaw =
		// 			std::min(0.f, camData.yaw + dt * camData.angleSpeed);
		// 	}
		// }
		camData.pitch = 30 - game.inputActions.camYRot * 30;

		// get the position the camera wants to be based on car position
		auto targetPos = camData.targetPosition(transform.pos, transform.rot);

		// interpolate between target and current position
		// if we are in an invalid position, just use the target position
		if (glm::all(glm::isnan(camData.position))) {
			camData.position = targetPos;
		} else {
			camData.position = targetPos * camData.posEasing +
							   camData.position * (1 - camData.posEasing);
		}

		// update camera for this object
		if (camData.camNumber >= MAX_CAMS) {
			dbug::log(
				"REND", 2,
				"Cam number %d is larger than the maximum number of cameras!",
				camData.camNumber);
		}

		// add cameras if there isn't enough
		while (cameras.size() < camData.camNumber + 1) {
			cameras.push_back(std::make_shared<CameraComp>());
		}
		cameras[camData.camNumber] = std::make_shared<CameraComp>(camData);

		dbug::log("REND", -1, "cam pos: [%f, %f, %f]", camData.position.x,
				  camData.position.y, camData.position.z);

		auto camVel = (camData.position - startLocation) * (1.0f / dt);

		// update the audio listner's frame and velocity for 3d audio
		game.audio->updateListenerVel(camVel.x, camVel.y, camVel.z);
		game.audio->updateListenerFrame(camData.GetViewMatrix());
	}
}
