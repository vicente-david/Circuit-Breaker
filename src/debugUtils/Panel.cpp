
#include "debugUtils/Logger.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <cstdio>
#include <string>

namespace dbugPanel {

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
}

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
	ImGui::Begin("Debug");

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
	ImGui::Begin("Vehicle Tuning");
	ImGui::InputText("Folder", &tuning::configFolder);
	ImGui::InputText("Base Conf.", &tuning::basePath);
	ImGui::InputText("Engine Conf.", &tuning::enginePath);
	tuning::reloadSpark = ImGui::Button("Reload Conf.");
	ImGui::SameLine();
	tuning::setFolder = ImGui::Button("Set Folder");

	ImGui::Checkbox("Show Colliders", &tuning::physicsShapes);
	ImGui::End();
}
void render() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// draw panels 
	ImGui::ShowDemoWindow();
	vehicleTuningPanel();
	debugPanel();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void cleanup() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

} // namespace dbugPanel
