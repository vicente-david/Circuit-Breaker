#include "LeaderboardSystem.h"
#include <glm/glm.hpp>
#include "debugUtils/Logger.h"
#include <algorithm>

std::shared_ptr<LeaderboardSystem> LeaderboardSystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	auto system = coord->registerSystem<LeaderboardSystem>();

	Signature sig;
	sig.set(coord->getComponentType<Leaderboard>());
	//sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<LapCounter>());

	coord->setSystemSignature<LeaderboardSystem>(sig);

	return system;
}

void LeaderboardSystem::update(GameState& game) {
	Leaderboard lb;
	lb.standings.clear();
	// iterate through all spark entities
	for (auto& entity : entities) {
		//Transform& eTransform = game.coordinator->getComponent<Transform>(entity);
		LapCounter& lapProg = game.coordinator->getComponent<LapCounter>(entity);
		float score = lapProg.currentLap * lapProg.progress;
		// store the entity and it's score.
		scores.push_back({score, entity});
	}

	std::sort(scores.begin(), scores.end(), std::greater<std::pair<float, Entity>>()); // sort entity positions in descending order

	// write the standings
	for (int i = 0; i < scores.size(); i++) {
		lb.standings.push_back(scores[i].second);
		//dbug::log("LEADERBOARD", 0, "Place (%d): %d", i + 1, lb.standings[i]);
	}
}