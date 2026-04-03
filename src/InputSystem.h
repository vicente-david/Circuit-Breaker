#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>


// we can have multiple different callbacks, for example define UI callbacks
// if we have multiple UIs we can have callbacks for that specific UI
// how we're going to abstract it, undecided as of right now
// right now it's just a class here...
// 
// Another thing was how to handle multiple callback events
// a suggested way was to layer
// for example: UI propagates input to the game layer, or UI stops propagating input at the UI layer
class CallbackInterface {

public:
	virtual void keyCallback(int key, int scancode, int action, int mods) {};
	virtual void mouseButtonCallback(int button, int action, int mods) {};
	virtual void cursorPositionCallback(double xpos, double ypos) {};
	virtual void scrollCallback(double xoffset, double yoffset) {};
	virtual void windowSizeCallback(int width, int height) {};
};

// might be offloaded to just containing purely inputs
// purely combined inputs
// that way we can offload the logic to something else and keep this
// as purely inputs
struct Actions {

	// actions are what the game actually uses
	// raw inputs get combined to make actions

	// actions
	// steer forward or backward (action)
	float moveForward = 0.0;
	float moveBackward = 0.0;

	// vehicle rotation (action)
	float xRotation = 0.0;
	float yRotation = 0.0;

	// camera rotation (action)
	float camXRot = 0.0;
	float camYRot = 0.0;


	// actions
	bool boost = false;
	bool boostJustPressed = false;
	bool shimmyRight = false;
	bool shimmyLeft = false;
	bool driftMode = false;
	bool respawn = false;

	// keyboar raw inputs to be ored with controller for full input
	bool kboost = false;
	bool kshimmyRight = false;
	bool kshimmyLeft = false;
	bool kdriftMode = false;
	bool kRespawn = false;

	bool reload = false;
	float cameraReset = true;
	bool lookBack = false;

	// raw inputs
	// controller inputs (raw input)
	float controllerForward = 0.0;
	float controllerBackward = 0.0;
	float controllerDir = 0.0;

	// keyboard inputs (raw input)
	bool keyboardForward = false;
	bool keyboardBackward = false;
	bool keyboardLookBack = false;
	
	// action
	float keyboardDir = 0.0;

	// camera rotation (raw input)
	float keyboardXRot = 0.0;
	float keyboardYRot = 0.0;
	float controllerXRot = 0.0;
	float controllerYRot = 0.0;



};

struct UIActions {

	// menu callbacks
	//
	// controls which menu is being displayed right now
	// -1 is main menu, 1 is normal gameplay (for now), 2 is paused
	int menuControl = -1;
	bool intializeGame = false;

	// UI navigation inputs (consumed each frame)
	bool navigateUp = false;    // move selection up
	bool navigateDown = false;  // move selection down
	bool confirm = false;       // confirm/activate selected button
	bool goBack = false;        // go back to previous menu (when pressing backspace or B on controller)

};


class InputSystem {

public:
	InputSystem();
	void attachWindow(GLFWwindow* window);
	// will abstract later to have arbitrary controls read from an external source, such as a .ini file
	// scan code input is probably better, since it is keyboard independent, can implement later
	void setCallback(std::shared_ptr<CallbackInterface> callbacks);
	const Actions& getActions();
	UIActions getUIActions(); // changed to a copy instead of a reference, so that we can clear certain flags
	void setMenuControl(int state); // lets external code change the menuControl value
	void combineInputs();
	void updateGamepad();


private:
	void attachCallbacks();
	GLFWwindow* window;
	std::shared_ptr<CallbackInterface> callbacks;
	GLFWgamepadstate controllerState;

	Actions actions;
	UIActions uiActions;

	// previous frame controller button states for edge detection (menu navigation)
	bool prevDpadUp = false;
	bool prevDpadDown = false;
	bool prevButtonA = false;
	bool prevButtonB = false;
	bool prevButtonStart = false;
	bool prevLeftStickUp = false;   // left stick pushed up past threshold
	bool prevLeftStickDown = false; // left stick pushed down past threshold

	// necessary for glfw
	static void keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionMetaCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void windowSizeMetaCallback(GLFWwindow* window, int width, int height);
};
