// store UI information that it just passes to the rendering system
// the intention is to have the UI system store
// various UI's and depdending on the game state, switch between them
// for example, when the player wins the game, the UI switches to displaying the win screen ui
// render this every frame but depending on the state of the game it renders something different

// we can just pass in the current game state
// for nested ui's for example pause menu + settings we can store via a stack

#pragma once
#include "UISystemComponents.h"
#include "../ecs/System.h"
#include "../ecs/Coordinator.h"
#include "../ecs/Component.h"
#include <glm/gtx/projection.hpp>


class UISystem : public System{
public:
	static std::shared_ptr<UISystem> registerSystem(std::shared_ptr<Coordinator>& coord); // ecs shenanigans
	
	TextUI raceUI();

	void update();

	TextUI changeToWinScreen(); // change to win screen when that event triggers

private:


};