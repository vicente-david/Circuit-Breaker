#include "RespawnSystem.h"
#include <glm/glm.hpp>
#include "debugUtils/Logger.h"

std::shared_ptr<RespawnSystem> RespawnSystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	auto system = coord->registerSystem<RespawnSystem>();

	Signature sig;
	sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<Respawnable>());
	sig.set(coord->getComponentType<LapCounter>());

	coord->setSystemSignature<RespawnSystem>(sig);
	
	return system;
}

void RespawnSystem::update(GameState& game) {
	// iterate through all entities in the game that are respawnable
	for (auto& entity : entities) {
		Transform& eTransform = game.coordinator->getComponent<Transform>(entity);

		if (eTransform.pos.y <= yDeadzone) {// if the entity fell beyond the respawn zone
			float currY = eTransform.pos.y;
			while (currY <)
			respawn(entity);
		}
	}
}

void RespawnSystem::respawn(Entity& entity) {
	Transform& t = game.coordinator
}