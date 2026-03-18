
#include "audio/AudioSystem.h"
#include "audio/AudioEngine.h"
#include "audio/Sound.h"

void AudioSystem::updateSounds(GameState &game) {

	for (auto const &entity : entities) {
		Sound &sound = game.coordinator->getComponent<Sound>(entity);
		game.audio->updateSoundLoc(sound);
		game.audio->updateSoundVel(sound);
	}
}

std::shared_ptr<AudioSystem>
AudioSystem::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<AudioSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<Sound>());
	coord->setSystemSignature<AudioSystem>(sig);

	return system;
}
