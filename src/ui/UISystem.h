// store UI information that it just passes to the rendering system
// the intention is to have the UI system store
// various UI's and depdending on the game state, switch between them
// for example, when the player wins the game, the UI switches to displaying the win screen ui
// render this every frame but depending on the state of the game it renders something different

// we can just pass in the current game state
// for nested ui's for example pause menu + settings we can store via a stack

// ok so how this is going to be structures as of right now:

// gamestate will push and pop screens, these screens will be defined and created here in the UISystem
// see the screens section below in the UiSystem class

// each UIScreen will contain a vector of UIElements
// ui elements will be considered as entities in our ECS system
// that contain the component UIElement

// to toggle visibility, we will use the screenStack vector (stack)
// this will display all currently active uiscreens
// we'll let UIsystem do it's own rendering
// but rendering system will have to pass some information 


// TLDR:
// Gamestate pushes/pops screens by name,  UISystem renders via stack, UIElements store the data

#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "../graphics/Text.h"
#include <string>
#include "UISystemComponents.h"
#include "../ecs/System.h"
#include "../ecs/Coordinator.h"
#include "../ecs/Component.h"
#include <glm/gtx/projection.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../GameState.h"


// positions of the triangle to render the quad
struct UIPositions {
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 p3;
	glm::vec3 p4;
	glm::vec3 p5;
	glm::vec3 p6;
};

// a UIscreen consists of a bunch of UIElements 
// we'll push screens into the UIstack
struct UIScreen {
	std::string name; // name of the ui screen
	std::vector<Entity> UIElements; // what elements make it up
};


class UISystem : public System{
public:
	static std::shared_ptr<UISystem> registerSystem(std::shared_ptr<Coordinator>& coord); // ecs shenanigans

	void update(std::string& fps);

	UIPositions calculateAnchorPositions(UIElement u1);

	void initializeRenderingParams(); // sets all necessary rendering params

	void renderUI();

	// 
	void addScreen(); // add a ui screen to the stack
	void popScreen(); // pop a ui screen from the stack 
	void clearAllScreens(); // clears all screens from the stack

	// initialization
	void screenInitialization(); // creates and stores all different ui screens

	// Screens (just means ui screens in general, not speficially limited to only pausemenu and mainmenu)
	// it could include things like a heads up display
	// fps counter
	// etc
	void createMainMenu(); // create the pause menu and push it to the hash map
	void createFPSCounter(); // create an fps counter
	void createLapCounter();
	void updateLapCounter(int lapcount);
	
	unsigned int uiVAO, uiVBO, textVBO, textVAO;

	std::map<char, Character> textFont;
	glm::mat4 textMat;

	glm::mat4 uiMat;

	GLFWwindow* window;

	std::unique_ptr<ShaderProgram> uiShader;
	std::unique_ptr<ShaderProgram> textProg;

	int* SCR_WIDTH; 
	int* SCR_HEIGHT;

private:

	std::vector<UIScreen> screenStack; // pretend this is a stack
	// we iterate backwards

	std::unordered_map<std::string, UIScreen> nameToScreen; // maps screen names to UIScreens

};