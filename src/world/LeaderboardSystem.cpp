#include "LeaderboardSystem.h"
#include <glm/glm.hpp>
#include "debugUtils/Logger.h"
#include <algorithm>
#include "vehicles/SparkComponents.h"

std::shared_ptr<LeaderboardSystem> LeaderboardSystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	auto system = coord->registerSystem<LeaderboardSystem>();

	Signature sig;
	sig.set(coord->getComponentType<Leaderboard>());
	//sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<LapCounter>());
	sig.set(coord->getComponentType<SparkData>());

	coord->setSystemSignature<LeaderboardSystem>(sig);

	return system;
}

void LeaderboardSystem::update(GameState& game) {
	scores.clear();

	// iterate through all spark entities
	for (auto& entity : entities) {
		LapCounter& lapProg = game.coordinator->getComponent<LapCounter>(entity);
		SparkData& sparkData = game.coordinator->getComponent<SparkData>(entity);
		const std::string& sparkName = sparkData.mVehicleName;
		float score = (lapProg.currentLap * 100000.0f) + lapProg.progress; // 100000.0f to ensure that lap count takes priority
		scores.push_back({score, sparkName});
	}

	std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) {
		return a.first > b.first; // sort by score descending
	});

	// write standings to each entity's Leaderboard component
	for (auto& entity : entities) {
		Leaderboard& lb = game.coordinator->getComponent<Leaderboard>(entity);
		lb.standings.clear();
		for (int i = 0; i < scores.size(); i++) {
			lb.standings.push_back(scores[i].second);
			//dbug::log("LEADERBOARD", 0, "Place (%d): %s (score: %.2f)", i + 1, scores[i].second.c_str(), scores[i].first);
		}
	}


}