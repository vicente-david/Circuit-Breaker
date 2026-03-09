// components of the UI still unsure how to do this
#pragma once
#include <glm/glm.hpp>
#include <string>

struct UIComponent { // if current UI screen is active
	bool isVisible;
};

struct activeUI {}; // empty struct to determine which UI(s) are active

struct RectUI {
	glm::vec2 pos; // screen space pos, corresponds to center of rectangle
	float width; // width in pixels
	float height; // height in pixels
	glm::vec3 col; // color
};

struct TextUI { // to do add textfont
	std::string textContent;
	float xPos;
	float yPos;
	float scale;
	glm::vec3 col;
};