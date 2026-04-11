#include "LeaderboardSystem.h"
#include <glm/glm.hpp>
#include "debugUtils/Logger.h"
#include <algorithm>
#include <random>
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
		float score = lapProg.currentLap * 100000 + lapProg.progress;
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
			dbug::log("LEADERBOARD", 0, "Place (%d): %s (score: %.2f)", i + 1, scores[i].second.c_str(), scores[i].first);
		}

		// lock in the finish order then randomize remaining (unfinished) AI.
		if (!finalPositionsLocked) {
			lb.finalPositions.clear(); // just to be safe
			for (auto& name : game.finishOrder) {
				lb.finalPositions.push_back(name);
			}

			// When the human player finishes, randomize remaining AI and lock it in
			if (game.nextState == END) {
				std::vector<std::string> remaining;
				for (auto& s : scores) {
					bool finished = false;
					for (auto& f : lb.finalPositions) {
						if (f == s.second) { finished = true; break; }
					}
					if (!finished) {
						remaining.push_back(s.second);
					}
				}

				static std::mt19937 rng(std::random_device{}()); // generate the random 'number', then shuffle the remaining AI
				std::shuffle(remaining.begin(), remaining.end(), rng); // shuffle function

				for (auto& name : remaining) {
					lb.finalPositions.push_back(name);
				}
				finalPositionsLocked = true;
			}
		}
	}

}

void LeaderboardSystem::reset() {
	finalPositionsLocked = false;
}
