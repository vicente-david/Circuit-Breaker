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
	scores.clear();

	// iterate through all spark entities
	for (auto& entity : entities) {
		LapCounter& lapProg = game.coordinator->getComponent<LapCounter>(entity);
		float score = lapProg.currentLap * lapProg.progress;
		scores.push_back({score, entity});
	}

	std::sort(scores.begin(), scores.end(), std::greater<std::pair<float, Entity>>());

	// write standings to each entity's Leaderboard component
	for (auto& entity : entities) {
		Leaderboard& lb = game.coordinator->getComponent<Leaderboard>(entity);
		lb.standings.clear();
		for (int i = 0; i < scores.size(); i++) {
			lb.standings.push_back(scores[i].second);
		}
	}

	//for (int i = 0; i < scores.size(); i++)
		//dbug::log("LEADERBOARD", 0, "Place (%d): Entity %d", i + 1, scores[i].second);

}