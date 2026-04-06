
#include "CameraSystem.h"
#include "GameState.h"
#include "PxQueryFiltering.h"
#include "PxQueryReport.h"
#include "PxRigidBody.h"
#include "debugUtils/Logger.h"
#include "foundation/PxVec3.h"
#include "geometry/PxGeometryHit.h"
#include "geometry/PxGeometryQuery.h"
#include "graphics/CameraComp.h"
#include "vehicles/SparkComponents.h"
#include <cstdio>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

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

float lerp(float val1, float val2, float factor) {
	return val1 * factor + val2 * (1 - factor);
}

void CameraSystem::update(GameState &game, float dt) {

	for (auto &entity : entities) {
		auto &sData = game.coordinator->getComponent<SparkData>(entity);
		auto &camData = game.coordinator->getComponent<CameraComp>(entity);
		auto &rBody = game.coordinator->getComponent<PxRigidBody *>(entity);
		auto &transform = game.coordinator->getComponent<Transform>(entity);

		// save the starting position so we can calculate velocity later
		auto startLocation = camData.position;

		camData.backwards = game.inputActions.lookBack;

		// lerp the yaw
		float targetYaw = game.inputActions.camXRot * 75;
		camData.yaw = lerp(targetYaw, camData.yaw, YAW_EASIING);

		// this can be used to move linearly, but i don't think it looks as good
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
		float targetfov = 45.f + glm::length(camVel) * 0.3;
		if (sData.isBoosting) {
			targetfov += 10;
			camData.posEasing = POS_EASIING_BOOST;
		} else {
			camData.posEasing = POS_EASIING;
		}

		camData.fov = lerp(targetfov, camData.fov, camData.fovEasing);
		camData.fov = std::min(100.f, camData.fov);

		// move the camera closer at faster speeds
		// this makes the camera closer to the ground, and compensates for some
		// of the fov changes
		camData.targetDist = 5.0 - (glm::length(camVel) * 0.03f);
		camData.targetDist = std::max(0.f, camData.targetDist);
		// printf("fov:%f dist:%f\n", camData.fov, camData.targetDist);

		// camCollision(game, camData,
		// 			 glm::length(camData.position - camData.lookPos));

		// update the audio listner's frame and velocity for 3d audio
		game.audio->updateListenerVel(camVel.x, camVel.y, camVel.z);
		game.audio->updateListenerFrame(camData.GetViewMatrix());
	}
}
// this doesnt work for no good reason
void CameraSystem::camCollision(GameState &game, CameraComp &camdata,
								float dist) {
	const PxVec3 origin(camdata.position.x, camdata.position.y,
						camdata.position.z);
	glm::vec3 glmDir = camdata.lookPos - camdata.position;
	glmDir = glm::normalize(glmDir);

	const PxVec3 dir(glmDir.x, glmDir.y, glmDir.z);
	PxRaycastBuffer hit;
	PxQueryFilterData filter(PxQueryFlag::eSTATIC);
	if (game.physics->gScene->raycast(origin, dir, dist, hit, PxHitFlag::eMTD,
									  filter)) {
		printf("blocking camera!!\n");
		printf("d:%f/%f\n", hit.block.distance, dist);
		printf("rom:[%f,%f,%f]\n", hit.block.normal.x, hit.block.normal.y,
			   hit.block.normal.z);
		printf("n:%d\n", hit.nbTouches);
		// camdata.position += glmDir * hit.block.distance*0.2f;
	}
}
