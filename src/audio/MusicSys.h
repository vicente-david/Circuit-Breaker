#pragma once

// idk if this is really a system because it only operates its 1 global music,
// but idk what else to call it. its like a system with 1 entity i guess

#include "GameState.h"
#include "audio/Music.h"
#include <string>
class MusicSys {
  public:
	Music music;
	GAMESTATE lastState = MAINMENU;
	void init(GameState &game);
	void update(float dt, GameState &game);
	void switchTrack(std::string intro, std::string loop, GameState &game);
};
