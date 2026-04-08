
#include "Music.h"
#include "AudioEngine.h"
#include "audio/Sound.h"
#include <cstdio>

Music::Music(std::shared_ptr<Sound> intro, std::shared_ptr<Sound> loop) {
	this->intro = intro;
	this->loop = loop;

	intro->setLooping(false);
}

// starts the sound so it will start playing
void Music::start() {
	intro->start();
}
void Music::update(float dt){
	if(intro->freed){
		loop->start();
	}
}
void Music::stop(){
	intro->stop();
	loop->stop();
}

