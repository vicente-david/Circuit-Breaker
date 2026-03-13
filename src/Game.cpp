#include "Game.h"

Game::Game() {
	coordinator = std::make_shared<Coordinator>();
	audio = std::make_shared<AudioEngine>();
}

void Game::initializeGame() {
	initializeECS(); //initialize ecs
	// initialize entities
	// construct track 
	// bake physics
}

void Game::initializeECS() {
	// create the coordinator
	// initialize coordinator
	coordinator->Init();

	// register components
	coordinator->registerComponent<Transform>();
	coordinator->registerComponent<physx::PxRigidBody*>();
	coordinator->registerComponent<Model>();
	coordinator->registerComponent<SparkControls>();
	coordinator->registerComponent<SparkData>();
	coordinator->registerComponent<HumanController>();
	coordinator->registerComponent<CameraComp>();
	coordinator->registerComponent<LapCounter>();
	coordinator->registerComponent<AIController>();
	coordinator->registerComponent<CollisionData>();
	coordinator->registerComponent<Sound>();
	coordinator->registerComponent<Respawnable>();
	coordinator->registerComponent<UIComponent>();
	coordinator->registerComponent<RectUI>();

	// register systems
	physicsSys = PhysicsSystem::registerSystem(coordinator);
	renderer = RenderingSystem::registerSystem(coordinator);
	sparkSys = SparkSys::registerSystem(coordinator);
	controllerSys = ControllerSys::registerSystem(coordinator);
	cameraSys = CameraSystem::registerSystem(coordinator);
	audioSys = AudioSystem::registerSystem(coordinator);
	aiControllerSys = AIControllerSys::registerSystem(coordinator);
	respawnSys = RespawnSystem::registerSystem(coordinator);
	lapSys = LapSystem::registerSystem(coordinator);
	uiSys = UISystem::registerSystem(coordinator);

}