#include "graphics/Camera.h"
#include "Entity.h"
#include "GLFW/glfw3.h"
#include "GameState.h"
#include "InputSystem.h"
#include "graphics/Model.h"
#include "physics/PhysicsSystem.h"
#include "PxPhysicsAPI.h"
#include "PxRigidDynamic.h"
#include "graphics/RenderingSystem.h"
#include "Vehicle.h"
#include "audio/AudioEngine.h"
#include "audio/Sound.h"
#include "debugUtils/Logger.h"
#include "ecs/Component.h"
#include "glad/gl.h"
#include <AL/al.h>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class Test1 : public System {
	int i;
};

int main() {
	// change to enable logging of different levels (0-> everything, 1->
	// warnings, 3-> errors
	dbug::minLogSeverity = 0;
	// dbug::logIgnore("PHYS");
	// dbug::logIgnore("ECS");
	// dbug::logIgnoreType = dbug::WHITE_LIST;

	dbug::loggerInit();

	GameState gameState;
	// create the coordinator
	// initialize coordinator
	gameState.coordinator->Init();

	// register components
	gameState.coordinator->registerComponent<Transform>();
	gameState.coordinator->registerComponent<physx::PxRigidDynamic*>();
	gameState.coordinator->registerComponent<Model>();

	// register systems
	auto testSystem = gameState.coordinator->registerSystem<Test1>();

	// create signature for the system
	Signature signature;
	signature.set(gameState.coordinator->getComponentType<Transform>());
	// set the signature
	gameState.coordinator->setSystemSignature<Test1>(signature);

	// physics system
	auto physicsSystem = PhysicsSystem::registerSystem(gameState.coordinator);
	auto renderer = RenderingSystem::registerSystem(gameState.coordinator);
	// gameState.physx = physicsSystem->gPhysics;


	Vehicle car1(physicsSystem);
	car1.init();
	car1.changeEngineDriveParams("TestDrive.json");

	InputSystem inputSystem;
	inputSystem.attachWindow(renderer->window);

	Actions gameActions = inputSystem.getActions();
	Camera c1 = Camera();

	// time
	double t = 0.0;
	const double dt = 1.0 / 60.0; // simulate at 60fps
	// if its slower than this, just slow down the game instead of lagging even
	// more
	const double minFps = 30.0;
	double currentTime = glfwGetTime();
	double accumulator = 0.0;

	renderer->initializeShaders(); // Create shader programs

	renderer->initializeText();

	// Create models
	Model cube("assets/cube.obj");
	Model spark("assets/spark.obj");
	Model track("assets/plane.obj"); // temporary track model

	// --Placeholder code: Add cubes to game state entityList
	gameState.entityList.reserve(465);
	// for (int i = 0; i < 465; i++) {
	// 	gameState.addEntity("perro cube", PhysType::RigidBody, &cube,
	// 						physicsSys.transformList[i]);
	// }
	gameState.addEntity("Spark", PhysType::Spark, &spark, &car1.transform);

	Transform none = {glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 0)};
	gameState.addEntity("Track", PhysType::StaticMesh, &track, &none);
	physicsSystem->initStaticMesh(track.GetMesh()[0], none);
	physicsSystem->createTestObjs(gameState.coordinator);

	// place holder test sounds
	Sound testSound = gameState.audio->createSound("muteCity");
	testSound.setLooping(true);
	testSound.start();
	float soundX = 0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	int framesPassed = 0;
	std::string fps = std::to_string(0);

	c1.Yaw = 0.0f;

	// RENDER LOOP
	while (!glfwWindowShouldClose(renderer->window)) {

		// time
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;
		accumulator = std::min(accumulator, 1 / minFps);
		framesPassed++;

		// input
		gameActions = inputSystem.getActions();
		car1.applyInput(gameActions, dt);

		/*std::cout << "Pos: " << tr.pos.x << ' ' << tr.pos.y << ' ' << tr.pos.z
		   << "\nRot: "
			<< tr.rot.x << ' ' << tr.rot.y << ' ' << tr.rot.z << '\n'
			<< std::endl;*/

		// physics
		while (accumulator >= dt) {
			car1.step(dt);
			physicsSystem->updatePhysics(dt, gameState);
			accumulator -= dt;
			t += dt;

			// dopler shift test
			// this stuff would  go in whatever is playing a sound (ex. physics
			// collision and like gamestate stuff for

			// test moving the sound left/right
			float soundVel = 0;
			if (gameActions.shimmyLeft) {
				soundX -= 0.5f;
				soundVel = -15;
				gameActions.shimmyLeft = false;
			}
			if (gameActions.shimmyRight) {
				soundX += 0.5f;
				soundVel = 15;
				gameActions.shimmyRight = false;
			}
			gameState.audio->updateSoundLoc(testSound, soundX, 0, 0);
			gameState.audio->updateSoundVel(testSound, soundVel, 0, 0);

			// dopler effect when boosting by setting listner's velocity
			if (gameActions.boost) {
				auto vel = glm::vec4(-10, 0, 0, 1) * c1.GetViewMatrix();
				gameState.audio->updateListenerVel(vel.x, vel.y, vel.z);

			} else {
				gameState.audio->updateListenerVel(0, 0, 0);
			}
		}

		if (t >= 1.0) {
			fps =
				std::to_string(static_cast<int>(std::round(framesPassed / t)));
			t -= 1.0;
			framesPassed = 0;
		}

		// c1.updateCamera(gameActions, accumulator);
		// todo: fix (will be integrated into new ecs system)
		c1.updateCamera(car1.transform.pos, car1.transform.forwardD,
						gameActions.camXRot, frameTime,
						gameActions.cameraReset);

		// rendering
		renderer->update(gameState, fps, c1);

		// update camera location in audio system for 3d audio
		gameState.audio->updateListenerFrame(c1.GetViewMatrix());
		// tihs just cleans up completed sounds, so it doesn't need to be called
		// super often
		gameState.audio->update(dt);
	}
	car1.cleanup();
	gameState.audio->close();
	glfwTerminate();

	return 0;
}
