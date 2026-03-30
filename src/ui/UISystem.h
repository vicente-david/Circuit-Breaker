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
#include <algorithm>
#include "../world/LeaderboardSystem.h"
#include "../vehicles/SparkComponents.h"
#include "../world/Clock.cpp"


// positions of the triangle to render the quad
struct UIPositions {
	std::vector<glm::vec3> points;
};

// a UIscreen consists of a bunch of UIElements 
// we'll push screens into the UIstack
struct UIScreen {
	std::string name; // name of the ui screen
	std::vector<Entity> UIElements; // what elements make it up

	std::vector<Entity> Buttons; // list of buttons that get animated (have an animatable component)
};

// contains all the data being passed to the uivbo
// contains both positions and colors/texture coords 
// if it has a bg color then the layout is (x,y,z,r,g,b) 
// if it has a texture then the layout is (x,y,z,u,v, unused) 
// because it is UIelements, z = 0
// the UIelement itself decides if it uses a bg color or a texture (it cannot do both)
struct UIVertex {
	glm::vec3 position;
	glm::vec3 color;
	float interpretFlag;
};

// same thing as above but used for mainly animated components
struct UIAnimVertex {
	glm::vec3 position;
	glm::vec3 color;
	float interpretFlag;
	glm::vec3 hLightColor;
};


class UISystem : public System {
public:
	static std::shared_ptr<UISystem> registerSystem(std::shared_ptr<Coordinator>& coord); // ecs shenanigans

	// iterate through active screens
	// and draw those ui elements
	// will call either updateSolidUI() if there is no texture
	// or updateTextureUI() if there is a texture
	// will not do both
	void update();
	void recalcMat();
	void updateUIElement(Entity& e); // renders UI using stored colors
	void updateAnimatedUIElement(Entity& e); // renders animated UI (will decide which shader to use for animated components)
	void updateButtonUIElement(Entity& e); // uses the button highlight shader to render the button

	UIPositions calculateAnchorPositions(UIElement u1); // calculates the quad coordinates of a container

	textPositions calculateTextContainer(UIElement u1); // exclusively used for defining a text container anchor

	void initializeRenderingParams(); // sets all necessary rendering params

	// add the screen to the screenstack
	// also update the uiData at the same time
	void addScreen(std::string screenName);
	void popScreen(); // pop a ui screen from the stack (and erase it's data from the uiData)
	void clearAllScreens(); // clears all screens from the stack (and clears the uiData)

	// initialization
	void screenInitialization(); // creates and stores all different ui screens

	// Screens (just means ui screens in general, not speficially limited to only pausemenu and mainmenu)
	// it could include things like a heads up display
	// fps counter
	// etc
	// the return value will be the name of the string it maps the screen to
	void createMainMenu(); // create the main menu and push it to the hash map
	void createPauseMenu(); // create the pause menu and push it to hashmap
	void createSettingsMenu(); // create the settings menu and push it to hashmap
	void createStandingsScreen(Leaderboard& lb); // create the standings menu and push it to hashmap (must be intialized separately)
	void createRacingHUD(); // create the racing hud and push it to the hashmap

	// persistent ui elements (elements that change every frame)
	void createFPSCounter(); // create an fps counter
	void updateFPSCounter();
	void createLapCounter(); // create the lap counter
	void updateLapCounter(int lapcount);
	void createPlaceCounter();
	void updatePlaceCounter(Entity& player);
	void createCountdown();
	void updateCountdown(std::string second, float time);
	Clock goTimer;
	bool go = true; // bool to toggle the "go" visibility

	void createBackwardsDisplay();
	void updateBackwardsDisplay(float time);
	
	void selectedEntities(); // mainly used for iterating through all visible screens and toggling the highlight flag

	unsigned int uiVAO, uiVBO, textVBO, textVAO;

	std::map<char, Character> textFont;

	glm::mat4 uiMat;

	GLFWwindow* window;

	std::unique_ptr<ShaderProgram> uiShader; // shader for ui elements that are colored 
	std::unique_ptr<ShaderProgram> textProg; // shader for text

	// instead of doing some funky passing, just keep a pointer to the correct information
	// just be cautious of what you're storing though
	// and ofc lifetimes
	// currently fps is a field of Game, so it should be ok
	// scr_width and scr_height are stored in the rendering system, it should be ok
	int* SCR_WIDTH;
	int* SCR_HEIGHT;
	std::string* fps;
	// entity based pointers will need a little more managment
	float* playerHealth;
	float* playerBoost;

	//
	bool* playerBackwards;
	Clock backwardClock = {2.0, 2.0}; // initialize a timer for going backwards display

	std::shared_ptr<Coordinator> coordinator;

	// animated component stuff

	unsigned int hlightVAO, hlightVBO;
	std::unique_ptr<ShaderProgram> hlightShader;
	// --- Button Selection ---
	int selectedButton = 0; // index of currently selected button (0-based, buttons only, excludes background. sliders TBD)

	// returns the number of selectable buttons on the top screen (excludes the background element at index 0)
	int getButtonCount();

	// returns the name of the top screen on the stack (empty string if no screens)
	std::string getTopScreenName();

	// resets selectedButton to 0 (call when switching screens)
	void resetSelection();

	// --- Button Selection End ---

private:

	int prevSCR_WIDTH = 0;
	int prevSCR_HEIGHT = 0;

	std::vector<UIScreen> screenStack; // pretend this is a stack
	std::vector<UIVertex> uiData;
	std::vector<UIAnimVertex> uiAnimData;

	// we iterate forwards since last element gets drawn on top

	// this could be useful
	// std::vector<UIElement> alwaysVisible; // a vector of always visible UI elements

	std::unordered_map<std::string, UIScreen> nameToScreen; // maps screen names to UIScreens

};