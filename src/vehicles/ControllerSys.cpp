
#include "vehicles/ControllerSys.h"
#include "GameState.h"
#include "InputSystem.h"
#include "ecs/Coordinator.h"
#include "vehicles/SparkComponents.h"
#include "ui/UISystem.h"
#include <iostream>

std::shared_ptr<ControllerSys>
ControllerSys::registerSystem(std::shared_ptr<Coordinator>& coord) {
	// register system
	auto system = coord->registerSystem<ControllerSys>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<SparkControls>());
	sig.set(coord->getComponentType<HumanController>());
	// sig.set(coord->getComponentType<physx::PxRigidDynamic *>());
	coord->setSystemSignature<ControllerSys>(sig);

	return system;
}

void ControllerSys::update(GameState& game) {

	// only handle UI input if we're in the main menu or paused states, settings menu is only accessible from these states
	if (game.currentState == MAINMENU || game.currentState == PAUSED || game.currentState == END)
		handleUINavigation(game);

	// --- Vehicle controls (only runs when player entities are spawned) ---
	for (auto const& entity : entities) {
		Actions& input = game.inputActions;
		SparkControls& sControl =
			game.coordinator->getComponent<SparkControls>(entity);

		SparkData& sData = game.coordinator->getComponent<SparkData>(entity);
		sControl.brake = input.moveBackward;
		sControl.driftMode = input.driftMode;
		sControl.throttle = input.moveForward;
		sControl.steering = input.xRotation;
		sControl.boost = input.boost;
		sControl.shimmyL = input.shimmyLeft;
		sControl.shimmyR = input.shimmyRight;
		sControl.reset = input.respawn;
		sControl.boost = input.boost;

		// I feel like this logic should not be in this file. Should move to SparkSys.cpp
		// activate health override when you start boosting with no boost left
		if (!input.boost) {
			sControl.boostWithHealth = false;
		} else if (input.boostJustPressed && sData.boost < 1.f) {
			sControl.boostWithHealth = true;
		}
		sControl.reload = input.reload;
	}
}

void ControllerSys::handleUINavigation(GameState& game) {
	UIActions& ui = game.uiActions;
	auto& uiSys = game.uiSystem;

	// safety: uiSystem may not be set yet during early initialization
	if (!uiSys) return;

	// --- Handle up/down to move selection ---
	if (ui.navigateUp) {
		ui.navigateUp = false;
		int buttonCount = uiSys->getButtonCount();
		if (buttonCount > 0) {
			uiSys->selectedButton--;
			if (uiSys->selectedButton < 0) {
				uiSys->selectedButton = buttonCount - 1; // wrap to bottom
			}
			uiSys->selectedEntities(); // tell the ui system to update what's currently selected
			std::cout << "[UI] Selected button: " << uiSys->selectedButton
				<< " on screen: " << uiSys->getTopScreenName() << std::endl;
		}
	}
	if (ui.navigateDown) {
		ui.navigateDown = false;
		int buttonCount = uiSys->getButtonCount();
		if (buttonCount > 0) {
			uiSys->selectedButton++;
			if (uiSys->selectedButton >= buttonCount) {
				uiSys->selectedButton = 0; // wrap to top
			}
			uiSys->selectedEntities(); // tell the ui system to update what's currently selected
			std::cout << "[UI] Selected button: " << uiSys->selectedButton
				<< " on screen: " << uiSys->getTopScreenName() << std::endl;
		}
	}

	// --- Handle backspace/B � go back from current menu ---
	if (ui.goBack) {
		ui.goBack = false;
		std::string topScreen = uiSys->getTopScreenName();

		if (topScreen == "settingsMenu") {
			std::cout << "[UI] Back from settings" << std::endl;
			uiSys->popScreen();
			uiSys->resetSelection();
		}
		else if (topScreen == "pauseMenu") {
			std::cout << "[UI] Resume game (back)" << std::endl;
			ui.menuControl = 1;
		}
		// mainMenu: back does nothing (nowhere to go back to)
	}

	// --- Handle confirm � map (screen, button) to an action ---
	if (ui.confirm) {
		ui.confirm = false;
		std::string topScreen = uiSys->getTopScreenName();
		int btn = uiSys->selectedButton;

		if (topScreen == "mainMenu") {
			// Buttons: 0=start game, 1=settings, 2=close game
			if (btn == 0) {
				std::cout << "[UI] Start game" << std::endl;
				ui.menuControl = 0;
				ui.intializeGame = true;
			}
			else if (btn == 1) {
				std::cout << "[UI] Open settings" << std::endl;
				uiSys->addScreen("settingsMenu");
				uiSys->resetSelection();
			}
			else if (btn == 2) {
				std::cout << "[UI] Exit game" << std::endl;
				glfwSetWindowShouldClose(uiSys->window, true);
			}
		}
		else if (topScreen == "pauseMenu") {
			// Buttons: 0=resume, 1=settings, 2=quit to menu
			if (btn == 0) {
				std::cout << "[UI] Resume game" << std::endl;
				uiSys->popScreen();
				ui.menuControl = 1;
				uiSys->resetSelection();
			}
			else if (btn == 1) {
				std::cout << "[UI] Open settings" << std::endl;
				uiSys->addScreen("settingsMenu");
				uiSys->resetSelection();
			}
			else if (btn == 2) {
				std::cout << "[UI] Quit to menu" << std::endl;
				uiSys->popScreen();
				ui.menuControl = -1;
				uiSys->addScreen("mainMenu");
			}
		}
		else if (topScreen == "settingsMenu") {
			// Buttons: 0=easy, 1=medium, 2=hard, 3=decorations, 4=particles, 5=back
			if (btn == 0) {
				// function to set AI difficulty to easy
				// TODO:
				// will need to account for restarting the game if a player chooses to change difficulty
				std::cout << "[UI] Difficulty: Easy" << std::endl;
			}
			else if (btn == 1) {
				std::cout << "[UI] Difficulty: Medium" << std::endl;
			}
			else if (btn == 2) {
				std::cout << "[UI] Difficulty: Hard" << std::endl;
			}
			else if (btn == 3) {
				std::cout << "[UI] Toggle track decorations" << std::endl;
			}
			else if (btn == 4) {
				std::cout << "[UI] Toggle VFX particles" << std::endl;
			}
			else if (btn == 5) {
				std::cout << "[UI] Back to previous menu" << std::endl;
				uiSys->popScreen();
				uiSys->resetSelection();
			}
		}
		else if (topScreen == "standingsScreen") {
			// buttons: 0=quit to menu, 1=restart game
			if (btn == 0) {
				std::cout << "[UI] Quit to menu" << std::endl;
				ui.menuControl = -1;
			}
			else if (btn == 1) {
				std::cout << "[UI] Restarting game" << std::endl;
				ui.intializeGame = true;
			}
		}
	}
}
