
#include "audio/Sound.h"
#include <al.h>
#include <memory>
#include <string>
class Music {
  public:
	Music() {}
	Music(std::shared_ptr<Sound> intro, std::shared_ptr<Sound> loop);

	void update(float dt);
	void start();
	void stop();
	void fadeOut();

	bool fading = false;
	std::shared_ptr<Sound> intro;
	std::shared_ptr<Sound> loop;
};
