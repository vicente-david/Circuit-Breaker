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

	// steer forward or backward (action)
	float moveForward = 0.0;
	float moveBackward = 0.0;

	// keyboard inputs (raw input)
	bool keyboardForward = false; 
	bool keyboardBackward = false;
	
	// action
	float keyboardDir = 0.0;

	// controller inputs (raw input)
	float controllerForward = 0.0;
	float controllerBackward = 0.0;
	float controllerDir = 0.0;

	// vehicle rotation (action)
	float xRotation = 0.0;
	float yRotation = 0.0;

	// camera rotation (action)
	float camXRot = 0.0;
	float camYRot = 0.0;

	// camera rotation (raw input)
	float keyboardXRot = 0.0;
	float controllerXRot = 0.0;

	// raw input
	float cameraReset = true;

	// actions
	bool boost = false;
	bool shimmyRight = false;
	bool shimmyLeft = false;
};


class InputSystem {

public:
	InputSystem();
	void attachWindow(GLFWwindow* window);
	// will abstract later to have arbitrary controls read from an external source, such as a .ini file
	// scan code input is probably better, since it is keyboard independent, can implement later
	void setCallback(std::shared_ptr<CallbackInterface> callbacks);
	const Actions& getActions();
	void combineInputs();
	void updateGamepad();


private:
	void attachCallbacks();
	GLFWwindow* window;
	std::shared_ptr<CallbackInterface> callbacks;
	GLFWgamepadstate controllerState;

	Actions actions;
	// necessary for glfw 
	static void keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionMetaCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void windowSizeMetaCallback(GLFWwindow* window, int width, int height);
};