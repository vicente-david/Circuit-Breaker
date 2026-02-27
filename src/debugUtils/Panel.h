
#include "GameState.h"
#include <string>
namespace dbugPanel {

// tuning values
namespace tuning {
extern bool reloadSpark;
extern bool setFolder;
extern std::string configFolder;
extern std::string enginePath;
extern std::string basePath;

} // namespace tuning

namespace debug {
	extern float updateTime;
	
}
void createPanel(GLFWwindow *window);
void frameStart();
void update(GameState &game);
void render();

void cleanup();
} // namespace dbugPanel
