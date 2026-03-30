
#include "debugUtils/Logger.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>
// #include <unistd.h>

struct SparkUI {
	int id;
	float health;
	float boost;
};

namespace dbugPanel {
// local variables
std::vector<SparkUI> sparkData;

// tuning values
namespace tuning {
bool reloadSpark = false;
bool setFolder = false;
bool physicsShapes;
std::string configFolder = "assets/vehicledata";
std::string enginePath = "SparkDrive.json";
std::string basePath = "SparkBase.json";

} // namespace tuning
namespace debug {
float updateTime;
float volume = 0.5;
bool updateVol = true;
} // namespace debug

void createPanel(GLFWwindow *window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |=
		ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |=
		ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(
		window, true); // Second param install_callback=true will install
					   // GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}
void debugPanel() {
	return; // REMOVE
	ImGui::Begin("Debug");

	float startV = debug::volume;
	ImGui::SliderFloat("Volume", &debug::volume, 0, 1);
	if (startV != debug::volume) {
		debug::updateVol = true;
	}
	ImGui::InputInt("Log level", &dbug::minLogSeverity);
	ImGui::Checkbox("Log whiteList", (bool *)&dbug::logListType);
	std::string tags = "Logging tags [";
	for (auto &t : dbug::logIgnoreTags) {
		tags += t;
	}
	tags += "]";
	ImGui::Text("%s", tags.c_str());
	ImGui::Text("Update time: %.2f ms (%.1f fps)", debug::updateTime * 1000,
				1 / debug::updateTime);
	ImGui::End();
}
void vehicleTuningPanel() {
	return; // REMOVE
	ImGui::Begin("Vehicle Tuning");
	ImGui::InputText("Folder", &tuning::configFolder);
	ImGui::InputText("Base Conf.", &tuning::basePath);
	ImGui::InputText("Engine Conf.", &tuning::enginePath);
	tuning::reloadSpark = ImGui::Button("Reload config");
	// ImGui::SameLine();
	// tuning::setFolder = ImGui::Button("Set Folder");
	// set path to <projdir>/assets/vehicledata instead of the build version
	if (ImGui::Button("Find project config folder")) {

		std::string path = std::filesystem::current_path().string();
		dbug::log(0, "CWD: %s", path.c_str());
		int idx = path.rfind("out");
		path = path.substr(0, idx) + "assets/vehicledata";
		tuning::configFolder = path;
	}

	ImGui::Checkbox("Show Colliders", &tuning::physicsShapes);
	ImGui::End();
}
void sparkInfo(int id, float health, float boost) {
	// reset if we are looping
	if(sparkData.size()>0 && sparkData[0].id==id){
		sparkData.clear();
	}
	sparkData.push_back(SparkUI{id, health, boost});
}
void clearSparkData() {
	sparkData.clear();
}
void drawSparkInfo() {
	return; // REMOVE
	if (sparkData.empty())
		return;

	// ---- Local player HUD (top-left corner) ----
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(220, 0));
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGui::Begin("Player HUD", nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar);

	auto& player = sparkData[0];

	// Health bar (green)
	ImGui::Text("Health");
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
		ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
	ImGui::ProgressBar(player.health / 100.0f, ImVec2(-1, 20));
	ImGui::PopStyleColor();

	ImGui::Spacing();

	// Boost bar (blue)
	ImGui::Text("Boost");
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
		ImVec4(0.2f, 0.5f, 1.0f, 1.0f));
	ImGui::ProgressBar(player.boost / 100.0f, ImVec2(-1, 20));
	ImGui::PopStyleColor();

	ImGui::End();
}

void render() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// draw panels
	// ImGui::ShowDemoWindow();
	vehicleTuningPanel();
	debugPanel();
	drawSparkInfo();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void cleanup() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

} // namespace dbugPanel
