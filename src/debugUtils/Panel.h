#pragma once

#include "GameState.h"
#include <string>
namespace dbugPanel {

extern bool enabled;
// tuning values
namespace tuning {
extern bool reloadSpark;
extern bool setFolder;
extern bool physicsShapes;
extern std::string configFolder;
extern std::string enginePath;
extern std::string basePath;

} // namespace tuning

namespace debug {
extern float updateTime;
extern float volume;
extern bool updateVol;

} // namespace debug
void createPanel(GLFWwindow *window);
void update(GameState &game);
void render();

void sparkInfo(int id, float health, float boost);
void clearSparkData();
void cleanup();
void startDebug();
} // namespace dbugPanel
