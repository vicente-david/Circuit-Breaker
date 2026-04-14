
#include "audio/Sound.h"
#include <al.h>
#include <memory>
class Music {
  public:
	Music() {}
	Music(std::shared_ptr<Sound> intro, std::shared_ptr<Sound> loop);

	void update(float dt);
	void start();
	void stop();
	void fadeOut();
	void volume(float vol);

	bool fading = false;
	float currVol = 1;
	float introVol = 1;
	std::shared_ptr<Sound> intro;
	std::shared_ptr<Sound> loop;
};
