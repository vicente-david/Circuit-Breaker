
#include "GameState.h"
#include "ecs/Coordinator.h"
#include "ecs/System.h"
#include <memory>

// handles playing sounds for the sparks
class SparkSoundSys : public System {

  public:
	static std::shared_ptr<SparkSoundSys>
	registerSystem(std::shared_ptr<Coordinator> &coord);

	// updates all the sparks in the game
	void updateSounds(double dt, GameState &gameState);
};
