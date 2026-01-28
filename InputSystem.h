#include <glad/gl.h>
#include <GLFW/glfw3.h>

class InputSystem {

public:
	InputSystem();
	void attachWindow(GLFWwindow* window);
	void attachCallbacks();
	// will abstract later to have arbitrary controls read from an external source, such as a .ini file
	// scan code input is probably better, since it is keyboard independent, can implement later
	virtual void keyCallback(int key, int scancode, int action, int mods);
	virtual void mouseButtonCallback(int button, int action, int mods);
	virtual void cursorPositionCallback(double xpos, double ypos);
	virtual void scrollCallback(double xoffset, double yoffset);
	virtual void windowSizeCallback(int width, int height);

private:

	GLFWwindow* window;
};