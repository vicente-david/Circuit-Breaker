#include "InputSystem.h"
#include<iostream>
#include<glm/glm.hpp>

class TestInput1 : public CallbackInterface {

	void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			actions->keyboardForward = true;
		}
		else if(key == GLFW_KEY_W && action == GLFW_RELEASE) {
			actions->keyboardForward = false;
		}

		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			actions->keyboardBackward = true;
		}
		else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			actions->keyboardBackward = false;
		}

		if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			actions->keyboardXRot = -1.0;
		}
		else if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
			actions->keyboardXRot = 0.0;
		}

		if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			actions->keyboardXRot = 1.0;
		}
		else if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
			actions->keyboardXRot = 0.0;
		}

		if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			actions->cameraReset = !actions->cameraReset;
		}


		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			actions->keyboardDir = -1.0;
		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			actions->keyboardDir = 0.0;
		}

		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			actions->keyboardDir = 1.0;
		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			actions->keyboardDir = 0.0;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			actions->boost = true;
		}
		else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			actions->boost = false;
		}

		if (key == GLFW_KEY_L) {
			if (action == GLFW_PRESS) {
				actions->shimmyRight = true;
			}
			else {
				actions->shimmyRight = false;
			}
		}

		if (key == GLFW_KEY_J) {
			if (action == GLFW_PRESS) {
				actions->shimmyLeft = true;
			}
			else {
				actions->shimmyLeft = false;
			}
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
// collect both controller and keyboard inputs
const Actions& InputSystem::getActions() {
	updateGamepad();
	combineInputs();
	return actions;
}

void InputSystem::combineInputs() {
	actions.moveForward = 0.0;
	actions.moveBackward = 0.0;
	actions.camXRot = 0.0;
	actions.xRotation = 0.0;


	// camera
	if (glm::abs(actions.controllerXRot) > glm::abs(actions.keyboardXRot))
		actions.camXRot = actions.controllerXRot;
	else
		actions.camXRot = actions.keyboardXRot;


	// steering
	if (glm::abs(actions.controllerDir) > glm::abs(actions.keyboardDir))
		actions.xRotation = actions.controllerDir;
	else
		actions.xRotation = actions.keyboardDir;
	


	if (actions.keyboardForward || actions.controllerForward) {
		actions.moveForward = glm::clamp((actions.keyboardForward+(float)actions.controllerForward), 0.0f, 1.0f);
	}

	if (actions.keyboardBackward || actions.controllerBackward) {
		actions.moveBackward = glm::clamp((actions.keyboardBackward + (float)actions.controllerBackward), 0.0f, 1.0f);
	}
}

void InputSystem::updateGamepad() {
	glfwGetGamepadState(GLFW_JOYSTICK_1, &controllerState);
	float triggerThreshold = 0.1f;
	float strickTriggerThreshold = 0.3f;
	float rightTrigger = controllerState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
	float leftTrigger = controllerState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
	float rightx = controllerState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
	float righty = controllerState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
	float leftx = controllerState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
	float lefty = controllerState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
	bool button_A = controllerState.buttons[GLFW_GAMEPAD_BUTTON_A];
	
	if (leftTrigger >= triggerThreshold) {
		actions.controllerBackward = leftTrigger;
	}else if (leftTrigger < triggerThreshold) {
		actions.controllerBackward = 0.0;
	}

	
	if (rightTrigger >= triggerThreshold) {
		actions.controllerForward = rightTrigger;
	} else if (rightTrigger < triggerThreshold) {
		actions.controllerForward = 0.0;
	}

	if (glm::abs(leftx) >= strickTriggerThreshold) {
		actions.controllerDir = -leftx;
	}
	else if (abs(leftx) < strickTriggerThreshold) {
		actions.controllerDir = 0.0;
	}


	if (glm::abs(rightx) >= strickTriggerThreshold)
		actions.controllerXRot = rightx;
	else if (abs(rightx) < strickTriggerThreshold)
		actions.controllerXRot = 0.0;

	if (!actions.boost) actions.boost = button_A;

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