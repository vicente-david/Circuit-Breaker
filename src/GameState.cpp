#include "GameState.h"
#include <algorithm>

GameState gameState;

Entity1* GameState::addEntity(std::string name, PhysType physType, Model* model, Transform* transform) {
	Entity1 e{ name, physType, model, transform };
	entityList.push_back(e);
	return &e;
}

//void GameState::removeEntity(Entity* e) {
//	entityList.erase(											// remove the entity from the list
//		std::remove_if(entityList.begin(), entityList.end(),	// scan from beginning to end
//		[e](const Entity& entity) { return &entity == e; }),	// if the pointer matches the obj, return an iterator
//		entityList.end()
//	);
//}

Entity1* GameState::findEntity(std::string name) {
	auto it = std::find_if(entityList.begin(), entityList.end(),
		[&](const Entity1& e) { return e.name == name; });

	if (it != entityList.end()) // if the entity is found
		return &(*it);			// return the pointer to the entity from entityList

	return nullptr;				// return nullptr if entity is not found
}

void GameState::endGame(Entity1* gameWinner) {
	winner = gameWinner;		// assign the game winner
								
								// TODO: probably do some UI/game management stuff here to give endscreen, etc.
								
	gameEnded = true;			// end the game
}

void GameState::resetGameState() {
	winner = nullptr;			// reset winner
	gameEnded = false;			// reset game ended to false

								// TODO: reset entityList (store 8 player entities). can be done after spark models ready
}
