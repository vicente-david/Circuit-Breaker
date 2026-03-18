#pragma once
#include "UISystem.h"

std::shared_ptr<UISystem> UISystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	// register system
	auto system = coord->registerSystem<UISystem>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<UIComponent>());
	sig.set(coord->getComponentType<RectUI>());

	//sig.set(coord->getComponentType<UIElement>());

	coord->setSystemSignature<UISystem>(sig);

	return system;
}

// called by rendering likely
void UISystem::update() {
	// we will assume game automatically updates which UI elements to show
	// so loop through active UI elements
	for (auto& entity : entities) {
		
	}
}

UIPositions UISystem::calculateAnchorPositions(UIElement u1, int width, int height) {
	UIPositions uF;  // ui triangle positions
	float left, top, right, bottom, leftO, topO, rightO, bottomO;

	// recall u1 stores  a vec4 of anchors in the following order (left, top, right, bottom)
	
	// 0,0 is top left corner

	left = u1.anchors.x; 
	top = u1.anchors.y; 
	right = u1.anchors.z;
	bottom = u1.anchors.w;

	// offsets
	leftO = u1.anchorOffsets.x;
	topO = u1.anchorOffsets.y;
	rightO = u1.anchorOffsets.z;
	bottomO = u1.anchorOffsets.w;

	// essentially the formula is very simple
	// bottom left corner is influenced by left and bottom right
	// so where is it located on left (left*width), and then offset it by leftOffset amount (leftO)
	// same thing for bottom, (bottom*height) + bottomOffset

	// start at bottom left then follow counter clockwise (bottom up) to build triangles
	uF.p1 = glm::vec3(left*width + leftO, bottom*height + bottomO, 0.0f); // (left, bottom, 0.0)
	uF.p2 = glm::vec3(right*width + rightO, bottom*height + bottomO, 0.0f); // (right, bottom, 0.0)
	uF.p3 = glm::vec3(right*width + rightO, top*height + topO, 0.0f); // (right, top, 0.0)

	uF.p4 = glm::vec3(left*width + leftO, bottom*height + bottomO, 0.0f); // (left, bottom, 0.0)
	uF.p5 = glm::vec3(right*width + rightO, top*height + topO, 0.0f); // (right, top, 0.0)
	uF.p6 = glm::vec3(left*width + leftO, top*height + topO, 0.0f); // (left, top, 0.0)
	
	return uF;
}


// raceUI should just be lap counter
// keep it simple for now
TextUI UISystem::raceUI(int lapcount) {
	// render text
	// render no rect
	TextUI text1;
	text1.textContent = "Current Lap: "+std::to_string(lapcount);
	text1.col = glm::vec3(1.0f);
	text1.scale = 1.0f;
	text1.xPos = 400.0f;
	text1.yPos = 1380.0f;

	return text1;
}

TextUI UISystem::changeToWinScreen() {
	// render text 
	// render a rect
	TextUI text1;
	text1.textContent = "You Win!";
	text1.col = glm::vec3(1.0f);
	text1.scale = 5.0f;
	text1.xPos = 300.0f;
	text1.yPos = 450.0f;
	
	return text1;
}

TextUI UISystem::changeToLoseScreen() {
	// render text
	// render no rect
	TextUI text1;
	text1.textContent = "You Lose!";
	text1.col = glm::vec3(1.0f);
	text1.scale = 5.0f;
	text1.xPos = 200.0f;
	text1.yPos = 1380.0f / 2;


	return text1;
}