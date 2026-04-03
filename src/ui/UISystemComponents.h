// components of the UI still unsure how to do this
#pragma once
#include <glm/glm.hpp>
#include <string>
#include "../graphics/Texture.h"

// align text enums
// will assume bottom left alignment by default
enum textAlign {
	CENTER,
	LEFT,
	RIGHT,
	TOP,
	BOTTOM
};

// enum to switch between shaders
enum animatedType {
	ANIM_BUTTON,
	ANIM_HEALTHBAR,
	ANIM_SPEEDOMETER
};

// where to place the text in pixel space
struct textPositions {
	float leftPx; // where is the left part of text container starting (typically ignored if using right align)
	float topPx; // where is the top part of text (typically ignored if using bottom align)
	float rightPx; // where is the right part of text (typically ignored if using left align)
	float bottomPx; // where is the bottom part of text (typically ignored if using top align)
	textAlign textAlignX = LEFT; // horizontal alignment enum
	textAlign textAlignY = TOP; // vertical alignment enum
};

// assume it's a quad
struct UIElement {
	bool isVisible; // visibility (currently unused)
	std::string text; //text centered by default
	float textScale; // scale of the text 
	glm::vec4 anchors = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); //left, top, right, bottom (default whole screen 0,0,1,1)
	glm::vec4 anchorOffsets = glm::vec4(0.0f); // left, top, right, bottom 

	glm::vec3 textColor;
	textAlign textAlignmentY;
	textAlign textAlignmentX; // anchros define the area of a ui element
	// text can behave indepdently of the container
	// we have three alignment types, center, left, and right
	// center will be assumed to be default
	// center meaning center relative to the anchors (parent container) 

	// anchors determine relative position

	// combined together with anchor offsets 
	std::string path; // path to texture
	unsigned int textureID; // id of texture
	// color of background (if no background this is used for texture coord)
	// texture coords will not use the z component of the vec3 
	// design texture to fill container, not the screen
	// Note: uv's are default initialized to the full texture
	glm::vec3 colors[6] = {
		glm::vec3(0.0f, 0.0f, 0.0f), // bottom left
		glm::vec3(1.0f, 0.0f, 0.0f), // bottom right
		glm::vec3(1.0f, 1.0, 1.0f), // top right
		glm::vec3(0.0, 0.0f, 0.0f), // bottom left
		glm::vec3(1.0, 1.0f, 1.0f), // top right
		glm::vec3(0.0f, 1.0f, 1.0f) // top left
	};
	bool hasBackgroundColor; // toggle bg color on/off

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


// attach to all UI elements
struct Animatable {
	bool isEnabled = true; // enabled by default

	bool isSelected = false; // false by default

	bool isHealth = false; // temu version of identifying if health bar 

	animatedType type = ANIM_BUTTON;  // edfault is a button

	glm::vec3 hLightColor = glm::vec3(1.0f); // additive highlight 
};