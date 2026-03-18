// store UI information that it just passes to the rendering system
// the intention is to have the UI system store
// various UI's and depdending on the game state, switch between them
// for example, when the player wins the game, the UI switches to displaying the win screen ui
// render this every frame but depending on the state of the game it renders something different

// we can just pass in the current game state
// for nested ui's for example pause menu + settings we can store via a stack

#pragma once
#include <string>
#include "UISystemComponents.h"
#include "../ecs/System.h"
#include "../ecs/Coordinator.h"
#include "../ecs/Component.h"
#include <glm/gtx/projection.hpp>


// positions of the triangle to render the quad
struct UIPositions {
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 p3;
	glm::vec3 p4;
	glm::vec3 p5;
	glm::vec3 p6;
};


class UISystem : public System{
public:
	static std::shared_ptr<UISystem> registerSystem(std::shared_ptr<Coordinator>& coord); // ecs shenanigans
	
	TextUI raceUI(int lapcount);

	void update();

	UIPositions calculateAnchorPositions(UIElement u1, int width, int height);

	TextUI changeToWinScreen(); // change to win screen when that event triggers
	TextUI changeToLoseScreen(); // change to win screen when that event triggers

private:


};