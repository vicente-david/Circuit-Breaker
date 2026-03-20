// components of the UI still unsure how to do this
#pragma once
#include <glm/glm.hpp>
#include <string>
#include "../graphics/Texture.h"

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

// align text enums
// will assume center align by default
enum textAlign {
	CENTER,
	LEFT,
	RIGHT
};

// assume it's a quad
struct UIElement {
	bool isVisible; // we'll load all of the ui at the start, and just toggle, instead of reconstructing
	std::string text; //text centered by default
	float textScale; // scale of the text 
	glm::vec4 anchors = glm::vec4(0.0f); //left, top, right, bottom
	glm::vec4 anchorOffsets = glm::vec4(0.0f); // left, top, right, bottom

	textAlign textAlignment; // anchros define the area of a ui element
	// text can behave indepdently of the container
	// we have three alignment types, center, left, and right
	// center will be assumed to be default
	// center meaning center relative to the anchors (parent container) 

	// anchors determine relative position

	// combined together with anchor offsets 
	std::string path; // path to texture
	glm::vec3 color; // solid fill bg color

	// anchors:
	// define where the corners go (ranges from 0 to 1))
	// left component defines where the left corner of the rect should start at
	// if 0, left corner starts at left side, if 1, left corner starts at right side
	// anywhere in between will position the left corner somewhere in between left and right
	
	// note these will also autoscale how big this ui element will be

	// for example: if left = 0.25 and right = 0.75
	// i've centered my ui element with a width of (0.75-0.25)*screenWidth
	// 
	// notice if left=0 and right = 1  then we get (1-0)*screenWidth = screenWidth
	// effectively stretching across from left to right
	// this makes sense because my left corner is set to the left side, and the right corner is set to the right side
	// so naturally it should span across the screen

	// top and bottom work the same way except vertically
	// 0 means top, 1 means bottom


	// the anchor offsets are pixel values
	// they allow slight modification 


	// consider html webpages, using width: 50% is akin to using left and right with a gap of 0.5
	// this is akin to doing something like left = 0, right = 0.5
	// aka relative to screensize
	// 
	// then consiedr width: 200px,  this is akin to doing something with anchorOffsets like left = 0, right = 200
	// fixed size


};