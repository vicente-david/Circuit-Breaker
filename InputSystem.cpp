#include "InputSystem.h"

// if we have .ini we could parse contorls here
InputSystem::InputSystem() : window(nullptr) {}

void InputSystem::attachWindow(GLFWwindow* w1) {
	window = w1;
}

void InputSystem::attachCallbacks() {

}

void InputSystem::keyCallback(int key, int scancode, int action, int mods)
{
}

void InputSystem::mouseButtonCallback(int button, int action, int mods)
{
}

void InputSystem::cursorPositionCallback(double xpos, double ypos)
{
}

void InputSystem::scrollCallback(double xoffset, double yoffset)
{
}

void InputSystem::windowSizeCallback(int width, int height)
{
}
