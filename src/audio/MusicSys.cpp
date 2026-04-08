
#include "MusicSys.h"
#include "GameState.h"

void MusicSys::init(GameState& game){
	auto intro = game.audio->createSound("death", false);
	auto loop = game.audio->createSound("title", false);
	music = Music(intro, loop);
	music.start();
}
void MusicSys::update(float dt, GameState &game) {
	// switch music based on game state
	if (lastState != game.currentState) {
		switch (game.currentState) {

		case MAINMENU:
			switchTrack("titleIntro", "title", game);

			break;
		case GAMEPLAY:
			switchTrack("muteCityIntro", "muteCityLoop",game);
			break;
		case PAUSED:
			break;
		case END:
			break;
		}
		lastState = game.currentState;
	}
	music.update(dt);
}
void MusicSys::switchTrack(std::string introName, std::string loopName, GameState& game) {
	music.stop();
	auto intro = game.audio->createSound(introName, false);
	auto loop = game.audio->createSound(loopName, false);
	music = Music(intro, loop);
	music.start();
}
