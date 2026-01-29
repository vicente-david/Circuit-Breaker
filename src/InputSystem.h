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

struct Actions {
	bool moveForward = false;
	bool moveBackward = false;
};


class InputSystem {

public:
	InputSystem();
	void attachWindow(GLFWwindow* window);
	// will abstract later to have arbitrary controls read from an external source, such as a .ini file
	// scan code input is probably better, since it is keyboard independent, can implement later
	void setCallback(std::shared_ptr<CallbackInterface> callbacks);
	const Actions& getActions();


private:
	void attachCallbacks();
	GLFWwindow* window;
	std::shared_ptr<CallbackInterface> callbacks;

	Actions actions;
	// necessary for glfw 
	static void keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionMetaCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void windowSizeMetaCallback(GLFWwindow* window, int width, int height);
};