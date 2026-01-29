#include "InputSystem.h"
#include<iostream>

class TestInput1 : public CallbackInterface {

	void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			std::cout << "W pressed" << std::endl;
			actions->moveForward = true;
		}
		else {
			actions->moveForward = false;
		}
	}

	void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
			std::cout << "left click" << std::endl;
		}
		else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE) {
			std::cout << "right click released" << std::endl;
		}
	}

	void cursorPositionCallback(double xpos, double ypos) {
		if (xpos > 50) {
			//std::cout << "x > 50" << std::endl;
		}
	}

	void scrollCallback(double xoffset, double yoffset) {
		if (yoffset < 0) {
			std::cout << "scroll down" << std::endl;
		}
	}

	void windowSizeCallback(int width, int height) {
		glViewport(0, 0, width, height);
	}
public:
	void setActions(Actions* actionsPtr) {
		actions = actionsPtr;
	}

private: 
		Actions* actions = nullptr;
};

// if we have .ini we could parse contorls here
InputSystem::InputSystem() : window(nullptr), callbacks(nullptr){
}

// attach a window
void InputSystem::attachWindow(GLFWwindow* w1) {
	window = w1;
	TestInput1 t1;
	t1.setActions(&actions);

	setCallback(std::make_shared<TestInput1>(t1));
}

// Note: when setting a new callback, it overwrites existing callback
// this enables iteration of callbacks by cycling through for example UI, Game, etc.
void InputSystem::setCallback(std::shared_ptr<CallbackInterface> callbacks_) {
	callbacks = callbacks_;
	attachCallbacks();
}

// attach all callbacks (if some are not defined it doesn't do anything)
void InputSystem::attachCallbacks() {
	// store the callback information in the window
	glfwSetWindowUserPointer(window, callbacks.get());

	// sets the active callbacks respectively
	glfwSetKeyCallback(window, keyMetaCallback);
	glfwSetMouseButtonCallback(window, mouseButtonMetaCallback);
	glfwSetCursorPosCallback(window, cursorPositionMetaCallback);
	glfwSetScrollCallback(window, scrollMetaCallback);
	glfwSetWindowSizeCallback(window, windowSizeMetaCallback);
}

// const as to never allow modification
const Actions& InputSystem::getActions() {
	return actions;
}

void InputSystem::keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// loads the callback information from the window (which we stored earlier)
	// essentially we nab the pointer from the window, cast this void* pointer into a pointer of CallbackInterface
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));

	// now we just set the callbacks we wish to use
	callbacks->keyCallback(key, scancode, action, mods);
	// this works because of something called vptrs and vtables (something exclusive to virtual functions)
	
}
void InputSystem::mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->mouseButtonCallback(button, action, mods);
}
void InputSystem::cursorPositionMetaCallback(GLFWwindow* window, double xpos, double ypos) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->cursorPositionCallback(xpos, ypos);
}
void InputSystem::scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->scrollCallback(xoffset, yoffset);
}
void InputSystem::windowSizeMetaCallback(GLFWwindow* window, int width, int height) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->windowSizeCallback(width, height);
}