#pragma once
#include "UISystem.h"

std::shared_ptr<UISystem> UISystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	// register system
	auto system = coord->registerSystem<UISystem>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<UIComponent>());
	sig.set(coord->getComponentType<RectUI>());

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