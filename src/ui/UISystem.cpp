#pragma once
#include "UISystem.h"

std::shared_ptr<UISystem> UISystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	// register system
	auto system = coord->registerSystem<UISystem>();
	// create system signture (what components this system needs)
	Signature sig;

	sig.set(coord->getComponentType<UIElement>());

	coord->setSystemSignature<UISystem>(sig);

	return system;
}

void UISystem::recalcMat() {
	// optimization recalc matrix only on window resize
	uiShader->use();
	uiMat = glm::ortho(0.0f, static_cast<float>(*SCR_WIDTH), static_cast<float>(*SCR_HEIGHT), 0.0f);
	glUniformMatrix4fv(glGetUniformLocation(uiShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat));
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat));
	hlightShader->use();
	glUniformMatrix4fv(glGetUniformLocation(hlightShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat));
}


void UISystem::updateUIElement(Entity& e) {
	uiShader->use();

	UIElement& u1 = coordinator->getComponent<UIElement>(e);
	uiData.clear();
	glBindVertexArray(uiVAO);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO);

	if (u1.hasBackgroundColor) {
		// if it has a bacground color 
		// then push the position, and the color
		UIPositions positions = calculateAnchorPositions(u1);
		// 6 points in a triangle based quad
		for (int i = 0; i < 6; i++) {
			UIVertex v1;
			v1.position = positions.points[i];
			v1.color = u1.colors[i];
			v1.interpretFlag = 0.0f;
			uiData.push_back(v1);
		}
		glBufferData(GL_ARRAY_BUFFER, uiData.size() * 7 * sizeof(float), uiData.data(), GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, uiData.size());
	}
	else if (!u1.path.empty()) {
		// render with texture
		UIPositions positions = calculateAnchorPositions(u1);
		// 6 points in a triangle based quad
		for (int i = 0; i < 6; i++) {
			UIVertex v1;
			v1.position = positions.points[i];
			v1.color = u1.colors[i];
			v1.interpretFlag = 1.0f;
			uiData.push_back(v1);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, u1.textureID);
		glBufferData(GL_ARRAY_BUFFER, uiData.size() * 7 * sizeof(float), uiData.data(), GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, uiData.size());

	}

	if (!u1.text.empty()) {
		textProg->use();
		textPositions p1 = calculateTextContainer(u1);
		RenderText(textProg->id, textVAO, textVBO, u1.text, p1, 1.0f, u1.textColor, textFont);
	}

}

// we only need to check the topmost screen stack items
// for right now only buttons can be highlighted
void UISystem::selectedEntities() {
	// return if screen stack is empty
	if (screenStack.empty()) return; 

	// if there exists a button
	if (getButtonCount() > 0) {
		// unhighlight all non selected buttons
		int i = 0;
		for (Entity& e : screenStack.back().UIElements) {

			if (!coordinator->hasComponent<Animatable>(e)) continue;

			if (i == selectedButton) {
				coordinator->getComponent<Animatable>(e).isSelected = true;
			}
			else {
				coordinator->getComponent<Animatable>(e).isSelected = false;
			}
			i++;
		}
	}

}

// basically the same as ui normal rendering except slightly different data
void UISystem::updateButtonUIElement(Entity& e) {
	hlightShader->use();

	UIElement& u1 = coordinator->getComponent<UIElement>(e);
	uiAnimData.clear();
	glBindVertexArray(hlightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, hlightVBO);

	if (u1.hasBackgroundColor) {
		// if it has a bacground color 
		// then push the position, and the color
		UIPositions positions = calculateAnchorPositions(u1);
		// 6 points in a triangle based quad
		for (int i = 0; i < 6; i++) {
			UIAnimVertex v1;
			v1.position = positions.points[i];
			v1.color = u1.colors[i];
			v1.interpretFlag = 0.0f;
			v1.hLightColor = coordinator->getComponent<Animatable>(e).hLightColor;
			uiAnimData.push_back(v1);
		}
		glBufferData(GL_ARRAY_BUFFER, uiAnimData.size() * 10 * sizeof(float), uiAnimData.data(), GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, uiAnimData.size());
	}
	else if (!u1.path.empty()) {
		// render with texture
		UIPositions positions = calculateAnchorPositions(u1);
		// 6 points in a triangle based quad
		for (int i = 0; i < 6; i++) {
			UIAnimVertex v1;
			v1.position = positions.points[i];
			v1.color = u1.colors[i];
			v1.interpretFlag = 1.0f;
			v1.hLightColor = coordinator->getComponent<Animatable>(e).hLightColor;
			uiAnimData.push_back(v1);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, u1.textureID);
		glBufferData(GL_ARRAY_BUFFER, uiAnimData.size() * 10 * sizeof(float), uiAnimData.data(), GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, uiAnimData.size());

	}

	if (!u1.text.empty()) {
		textProg->use();
		textPositions p1 = calculateTextContainer(u1);
		RenderText(textProg->id, textVAO, textVBO, u1.text, p1, 1.0f, u1.textColor, textFont);
	}

}

// assume everything is updated already
void UISystem::updateAnimatedUIElement(Entity& e) {
	Animatable& animComp = coordinator->getComponent<Animatable>(e);
	switch (animComp.type) {
	case(ANIM_BUTTON):
		// if not selected run the normal uielement shader 
		if (!animComp.isSelected) {
			updateUIElement(e);
		} else {
			updateButtonUIElement(e);
		}
		break;
	case(ANIM_HEALTHBAR):
		break;
	case(ANIM_SPEEDOMETER):
		break;
	}
}


// called by rendering likely
void UISystem::update() {
	// we will assume game automatically updates which UI elements to show
	// so loop through active UI elements

	// disable depth test, only render in order
	glDisable(GL_DEPTH_TEST);

	// detect window resize by comparing previous windowSize and now window size
	if (!(*SCR_WIDTH == prevSCR_WIDTH) || !(*SCR_HEIGHT == prevSCR_HEIGHT)) {
		recalcMat(); // recalc ortho projection matrix when screen size changes
		prevSCR_WIDTH = *SCR_WIDTH;
		prevSCR_HEIGHT = *SCR_HEIGHT;
	}

	// for all screens render the containers and the text
	for (UIScreen& u1 : screenStack) {
		for (Entity& e : u1.UIElements) {
			// two cases
			// static ui 
			// animated ui

			// if does not have the animatable component, do normal ui updates
			if (!coordinator->hasComponent<Animatable>(e))
				updateUIElement(e);
			else {
				updateAnimatedUIElement(e);
			}
		}
	}


	// render text after the ui elements
	//updateText();

	// re enable depth testing for 3d scenes
	glEnable(GL_DEPTH_TEST);
}

UIPositions UISystem::calculateAnchorPositions(UIElement u1) {
	int width = *SCR_WIDTH;
	int height = *SCR_HEIGHT;
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
	uF.points.push_back(glm::vec3(left * width + leftO, bottom * height + bottomO, 0.0f)); // (left, bottom, 0.0)
	uF.points.push_back(glm::vec3(right * width + rightO, bottom * height + bottomO, 0.0f)); // (right, bottom, 0.0)
	uF.points.push_back(glm::vec3(right * width + rightO, top * height + topO, 0.0f)); // (right, top, 0.0)

	uF.points.push_back(glm::vec3(left * width + leftO, bottom * height + bottomO, 0.0f)); // (left, bottom, 0.0)
	uF.points.push_back(glm::vec3(right * width + rightO, top * height + topO, 0.0f)); // (right, top, 0.0)
	uF.points.push_back(glm::vec3(left * width + leftO, top * height + topO, 0.0f)); // (left, top, 0.0)

	return uF;
}

textPositions UISystem::calculateTextContainer(UIElement u1) {
	textPositions tp1;
	tp1.leftPx = u1.anchors.x * (*SCR_WIDTH) + u1.anchorOffsets.x;
	tp1.topPx = u1.anchors.y * (*SCR_HEIGHT) + u1.anchorOffsets.y;
	tp1.rightPx = u1.anchors.z * (*SCR_WIDTH) + u1.anchorOffsets.z;
	tp1.bottomPx = u1.anchors.w * (*SCR_HEIGHT) + u1.anchorOffsets.w;
	tp1.textAlignX = u1.textAlignmentX;
	tp1.textAlignY = u1.textAlignmentY;
	return tp1;
}

void UISystem::initializeRenderingParams() {
	textProg = std::make_unique<ShaderProgram>("shaders/testText.vert", "shaders/testText.frag");

	// ui initialization
	uiShader = std::make_unique <ShaderProgram>("shaders/ui.vert", "shaders/ui.frag"); // upd ui shader ptr

	uiMat = glm::ortho(0.0f, static_cast<float>(*SCR_WIDTH), static_cast<float>(*SCR_HEIGHT), 0.0f); // create iniital ortho projection
	uiShader->use(); // use it first
	glUniformMatrix4fv(glGetUniformLocation(uiShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat)); // upload the uniform



	glGenVertexArrays(1, &uiVAO);
	glBindVertexArray(uiVAO);

	glGenBuffers(1, &uiVBO);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO);

	// modify as needed

	// currently 7 floats (x,y,z,r,g,b, textFlag) per point
	// 6 total because that's how many you need for a quad (2 triangles)
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * 6, NULL, GL_DYNAMIC_DRAW);

	// first position is just (x,y, z) but z=0
	// position is mapped to layout = 0, size is 3 since (x,y,z) the stride is 7 because [x,y,z,r,g,b, flag, x] skip 6 for the next starting point
	// and it starts at 0 so (void*)0
	// color is mapped to layout = 1 size is 3 since (r,g,b) the stride is 7 again [r,g,b,x,y,z,flag,r] skips 7 to get to the next starting point
	// and it starts after 3 floats so (void*)(3*sizeof(float)

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	// starts after 6 floats 
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));

	// enable all
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	// Initialize and bind VAO
	glGenVertexArrays(1, &textVAO);
	glBindVertexArray(textVAO);
	// Generate VBO
	glGenBuffers(1, &textVBO);
	// bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	textFont = initFont("assets/PixelifySans-VariableFont_wght.ttf");
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1,
		GL_FALSE, glm::value_ptr(uiMat));


	// highlight shadder stuff
	hlightShader = std::make_unique<ShaderProgram>("shaders/uiHighlight.vert", "shaders/uiHighlight.frag");

	hlightShader->use();

	glUniformMatrix4fv(glGetUniformLocation(hlightShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat)); // upload the uniform
	glGenVertexArrays(1,  &hlightVAO);
	glBindVertexArray(hlightVAO);

	glGenBuffers(1, &hlightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, hlightVBO);


	// 10 floats, 6 things for a quad
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 10 * 6, NULL, GL_DYNAMIC_DRAW);

	// position, col/texcoord, flag, (additive) highlight color,
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));


	// enable all
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	// vao layout is vec3, vec3, float, vec3



}

void UISystem::addScreen(std::string screenName) {
	// retrieve the screen's uielements
	UIScreen u1 = nameToScreen[screenName];
	// push it into active screens
	screenStack.push_back(u1);
	resetSelection(); // new screen on top, reset selection
}

void UISystem::popScreen() {
	UIScreen u1 = screenStack.back();
	screenStack.pop_back();
	resetSelection(); // screen below is now active, reset selection
}

void UISystem::clearAllScreens() {
	screenStack.clear();
	resetSelection();
}
// ---
int UISystem::getButtonCount() {
	if (screenStack.empty()) 
		return 0;
	// the top screen's first element (index 0) is always the background, rest are buttons
	int totalElements = (int)screenStack.back().UIElements.size();
	return std::max(0, totalElements - 1); // subtract background
}

std::string UISystem::getTopScreenName() {
	if (screenStack.empty()) 
		return "";
	return screenStack.back().name;
}

void UISystem::resetSelection() {
	selectedButton = 0;
	selectedEntities(); // update selected components
}
// ---
void UISystem::screenInitialization() {
	createFPSCounter();
	createMainMenu();
	createPauseMenu();
	createSettingsMenu();
	//createStandingsScreen(); initialized seperately)
	createRacingHUD();
	createLapCounter();
	createPlaceCounter();
}

// Recall UIElement has the following fields
/*
std::string text;
float textScale;
glm::vec4 anchors = glm::vec4(0.0f);
glm::vec4 anchorOffsets = glm::vec4(0.0f);
glm::vec3 textColor;
textAlign textAlignmentY;
textAlign textAlignmentX;
std::string path;
glm::vec3 color;
*/

// return the name of the screenName
void UISystem::createFPSCounter() {

	UIElement counter1;
	counter1.text = "FPS: " + *fps;
	counter1.textScale = 1.0f;
	// default anchors are whole screen (0,0,1,1)
	counter1.textColor = glm::vec3(1.0f);
	counter1.textAlignmentX = RIGHT;
	counter1.textAlignmentY = TOP;

	counter1.hasBackgroundColor = false;

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, counter1);

	UIScreen fpsCounter;
	fpsCounter.name = "fpsCounter";
	fpsCounter.UIElements.push_back(e1);

	nameToScreen["fpsCounter"] = fpsCounter;
}

void UISystem::updateFPSCounter() {
	// can assume it's only the first thing (we hard coded it above)
	Entity& e1 = nameToScreen["fpsCounter"].UIElements[0];
	UIElement& u1 = coordinator->getComponent<UIElement>(e1);
	u1.text = "FPS: " + *fps;
}

void UISystem::createPlaceCounter() {
	UIElement counter1;
	counter1.text = "Position: " + std::to_string(0);
	counter1.textScale = 1.0f;
	// default anchors are whole screen (0,0,1,1)
	counter1.textColor = glm::vec3(1.0f);
	counter1.textAlignmentX = RIGHT;
	counter1.textAlignmentY = BOTTOM;

	counter1.hasBackgroundColor = false;

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, counter1);

	UIScreen placeCounter;
	placeCounter.name = "placeCounter";
	placeCounter.UIElements.push_back(e1);

	nameToScreen["placeCounter"] = placeCounter;
}


// assume the player is being passed
void UISystem::updatePlaceCounter(Entity& p) {
	Entity& e1 = nameToScreen["placeCounter"].UIElements[0];
	UIElement& u1 = coordinator->getComponent<UIElement>(e1);

	Leaderboard& lb = coordinator->getComponent<Leaderboard>(p);
	int placement = 0;
	for (int i = 0; i < lb.standings.size(); i++) {
		if (coordinator->getComponent<SparkData>(p).mVehicleName == lb.standings[i]) {
			placement = i;
			break;
		}
	}

	u1.text = "Position: " + std::to_string(placement+1);
}

void UISystem::createMainMenu() {
	UIElement menu1;
	//menu1.text = "WOAH THE MENU CAN HAVE INDEPENDENT TEXT CRAZY!";
	//menu1.textScale = 1.0f;
	// default anchors are whole screen (0,0,1,1)
	//menu1.textColor = glm::vec3(1.0f);
	//menu1.textAlignmentX = CENTER;
	//menu1.textAlignmentY = BOTTOM;

	menu1.hasBackgroundColor = false;

	menu1.path = "assets/textures/ui/mainMenu/startmenu_bg.png";
	menu1.textureID = GenerateTexture(menu1.path.c_str(), false);

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, menu1);

	// --- BUTTONS ---

	// start game button
	UIElement startGameButton;
	startGameButton.hasBackgroundColor = false;
	startGameButton.path = "assets/textures/ui/mainMenu/startmenu_startgame.png";
	startGameButton.textureID = GenerateTexture(startGameButton.path.c_str(), false);

	startGameButton.anchors = glm::vec4(0.3, 0.56298, 0.7, 0.6444);
	startGameButton.anchorOffsets = glm::vec4(0, -12, 0, -12);

	// exit button 
	UIElement exitButton;
	exitButton.hasBackgroundColor = false;
	exitButton.path = "assets/textures/ui/mainMenu/startmenu_exitgame.png";
	exitButton.textureID = GenerateTexture(exitButton.path.c_str(), false);
	exitButton.anchors = glm::vec4(0.3, 0.7525, 0.7, 0.8325);

	// settings button
	UIElement settingsButton;
	settingsButton.hasBackgroundColor = false;
	settingsButton.path = "assets/textures/ui/mainMenu/startmenu_settings.png";
	settingsButton.textureID = GenerateTexture(settingsButton.path.c_str(), false);

	settingsButton.anchors = glm::vec4(0.3, 0.66539, 0.7, 0.74682);
	settingsButton.anchorOffsets = glm::vec4(0, -12, 0, -12);

	// --- ---

	Entity e2 = coordinator->createEntity();
	coordinator->addComponent(e2, startGameButton); // button 0
	coordinator->addComponent(e2, Animatable());

	Entity e3 = coordinator->createEntity();
	coordinator->addComponent(e3, settingsButton); // button 1
	coordinator->addComponent(e3, Animatable());

	Entity e4 = coordinator->createEntity();
	coordinator->addComponent(e4, exitButton); // button 2
	coordinator->addComponent(e4, Animatable());

	UIScreen mainMenu;
	mainMenu.name = "mainMenu";
	mainMenu.UIElements.push_back(e1);

	mainMenu.UIElements.push_back(e2);
	mainMenu.UIElements.push_back(e3);
	mainMenu.UIElements.push_back(e4);

	nameToScreen["mainMenu"] = mainMenu;
}

void UISystem::createPauseMenu() {
	UIElement menu1;
	// default anchors are whole screen (0,0,1,1)

	menu1.hasBackgroundColor = false; // render texture

	menu1.path = "assets/textures/ui/pauseMenu/pause_bg.png";
	menu1.textureID = GenerateTexture(menu1.path.c_str(), false);

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, menu1);

	// --- BUTTONS --

	// resume button
	UIElement resumeButton;
	resumeButton.hasBackgroundColor = false;
	resumeButton.path = "assets/textures/ui/pauseMenu/pause_resume.png";
	resumeButton.textureID = GenerateTexture(resumeButton.path.c_str(), false);
	resumeButton.anchors = glm::vec4(0.3, 0.6628, 0.7, 0.7443);
	resumeButton.anchorOffsets = glm::vec4(0, -168, 0, -168);

	// settings button
	UIElement settingsButton;
	settingsButton.hasBackgroundColor = false;
	settingsButton.path = "assets/textures/ui/pauseMenu/pause_settings.png";
	settingsButton.textureID = GenerateTexture(settingsButton.path.c_str(), false);
	settingsButton.anchors = glm::vec4(0.3, 0.6628, 0.7, 0.7443);
	settingsButton.anchorOffsets = glm::vec4(0, -84, 0, -84);

	// exit button 
	UIElement exitButton;
	exitButton.hasBackgroundColor = false;
	exitButton.path = "assets/textures/ui/pauseMenu/pause_quittomenu.png";
	exitButton.textureID = GenerateTexture(exitButton.path.c_str(), false);
	exitButton.anchors = glm::vec4(0.3, 0.6628, 0.7, 0.7443);

	// --- ---

	Entity e2 = coordinator->createEntity();
	coordinator->addComponent(e2, resumeButton); // button 0
	coordinator->addComponent(e2, Animatable());

	Entity e3 = coordinator->createEntity();
	coordinator->addComponent(e3, settingsButton); // button 1
	coordinator->addComponent(e3, Animatable());

	Entity e4 = coordinator->createEntity();
	coordinator->addComponent(e4, exitButton); // button 2
	coordinator->addComponent(e4, Animatable());


	UIScreen pauseMenu;
	pauseMenu.name = "pauseMenu";
	pauseMenu.UIElements.push_back(e1);

	pauseMenu.UIElements.push_back(e2);
	pauseMenu.UIElements.push_back(e3);
	pauseMenu.UIElements.push_back(e4);

	nameToScreen["pauseMenu"] = pauseMenu;
}

void UISystem::createSettingsMenu() {
	UIElement menu1;
	// default anchors are whole screen (0,0,1,1)

	menu1.hasBackgroundColor = false; // render texture

	menu1.path = "assets/textures/ui/settings/settings_bg.png";
	menu1.textureID = GenerateTexture(menu1.path.c_str(), false);

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, menu1);

	// --- BUTTONS ---

	// easy button
	UIElement easyButton;
	easyButton.hasBackgroundColor = false;
	easyButton.path = "assets/textures/ui/settings/diff_easy.png";
	easyButton.textureID = GenerateTexture(easyButton.path.c_str(), false);
	easyButton.anchors = glm::vec4(0.5313, 0.3473, 0.64396, 0.3982);

	// medium button
	UIElement mediumButton;
	mediumButton.hasBackgroundColor = false;
	mediumButton.path = "assets/textures/ui/settings/diff_medium.png";
	mediumButton.textureID = GenerateTexture(mediumButton.path.c_str(), false);
	mediumButton.anchors = glm::vec4(0.5313, 0.3473, 0.64396, 0.3982);
	mediumButton.anchorOffsets = glm::vec4(144, 0, 144, 0);

	// hard button
	UIElement hardButton;
	hardButton.hasBackgroundColor = false;
	hardButton.path = "assets/textures/ui/settings/diff_hard.png";
	hardButton.textureID = GenerateTexture(hardButton.path.c_str(), false);
	hardButton.anchors = glm::vec4(0.5313, 0.3473, 0.64396, 0.3982);
	hardButton.anchorOffsets = glm::vec4(288, 0, 288, 0);

	// track decorations button
	UIElement decorationsButton;
	decorationsButton.hasBackgroundColor = false;
	decorationsButton.path = "assets/textures/ui/settings/settings_trackdecorations.png";
	decorationsButton.textureID = GenerateTexture(decorationsButton.path.c_str(), false);
	decorationsButton.anchors = glm::vec4(0.1172, 0.7163, 0.495, 0.7672);

	// vfx particles button
	UIElement particlesButton;
	particlesButton.hasBackgroundColor = false;
	particlesButton.path = "assets/textures/ui/settings/settings_vfxparticles.png";
	particlesButton.textureID = GenerateTexture(particlesButton.path.c_str(), false);
	particlesButton.anchors = glm::vec4(0.1172, 0.7163, 0.495, 0.7672);
	particlesButton.anchorOffsets = glm::vec4(465, 0, 465, 0);

	// back to menu button
	UIElement menuButton;
	menuButton.hasBackgroundColor = false;
	menuButton.path = "assets/textures/ui/settings/settings_backtomenu.png";
	menuButton.textureID = GenerateTexture(menuButton.path.c_str(), false);
	menuButton.anchors = glm::vec4(0.0935, 0.8206, 0.9073, 0.902);

	// --- ---

	Entity e2 = coordinator->createEntity();
	coordinator->addComponent(e2, easyButton); // button 0
	coordinator->addComponent(e2, Animatable());

	Entity e3 = coordinator->createEntity();
	coordinator->addComponent(e3, mediumButton); // button 1
	coordinator->addComponent(e3, Animatable());

	Entity e4 = coordinator->createEntity();
	coordinator->addComponent(e4, hardButton); // button 2
	coordinator->addComponent(e4, Animatable());

	Entity e5 = coordinator->createEntity();
	coordinator->addComponent(e5, decorationsButton); // button 3
	coordinator->addComponent(e5, Animatable());

	Entity e6 = coordinator->createEntity();
	coordinator->addComponent(e6, particlesButton); // button 4
	coordinator->addComponent(e6, Animatable());

	Entity e7 = coordinator->createEntity();
	coordinator->addComponent(e7, menuButton); // button 5
	coordinator->addComponent(e7, Animatable());


	UIScreen settingsMenu;
	settingsMenu.name = "settingsMenu";
	settingsMenu.UIElements.push_back(e1);

	settingsMenu.UIElements.push_back(e2);
	settingsMenu.UIElements.push_back(e3);
	settingsMenu.UIElements.push_back(e4);
	settingsMenu.UIElements.push_back(e5);
	settingsMenu.UIElements.push_back(e6);
	settingsMenu.UIElements.push_back(e7);

	nameToScreen["settingsMenu"] = settingsMenu;
}

void UISystem::createStandingsScreen(Leaderboard& lb) {
	UIElement menu1;
	// default anchors are whole screen (0,0,1,1)

	menu1.hasBackgroundColor = false; // render texture

	menu1.path = "assets/textures/ui/standings/standings_bg.png";
	menu1.textureID = GenerateTexture(menu1.path.c_str(), false);

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, menu1);

	UIScreen standingsScreen;
	standingsScreen.name = "standingsScreen";
	standingsScreen.UIElements.push_back(e1);

	// --- POSITIONS FIELDS ---
	for (int i = 0; i < lb.standings.size(); i++) {
		UIElement firstPlace;
		firstPlace.hasBackgroundColor = false;
		firstPlace.path = "assets/textures/ui/standings/standings_position.png";
		firstPlace.textureID = GenerateTexture(firstPlace.path.c_str(), false);
		firstPlace.anchors = glm::vec4(0.3225, 0.145, 0.6785, 0.2252);
		firstPlace.anchorOffsets = glm::vec4(0, 66*i, 0, 66*i);
		firstPlace.text = " " + std::to_string(i+1) + ". "+lb.standings[i];
		firstPlace.textAlignmentY = CENTER;
		firstPlace.textAlignmentX = LEFT;
		firstPlace.textColor = glm::vec3(1.0f);
		Entity e2 = coordinator->createEntity();
		coordinator->addComponent<UIElement>(e2, firstPlace);
		standingsScreen.UIElements.push_back(e2);
	}


	// --- BUTTONS ---

	// back to menu button
	UIElement menuButton;
	menuButton.hasBackgroundColor = false;
	menuButton.path = "assets/textures/ui/standings/standings_backtomenu.png";
	menuButton.textureID = GenerateTexture(menuButton.path.c_str(), false);
	menuButton.anchors = glm::vec4(0.3225, 0.833, 0.485, 0.9148);

	// restart game button
	UIElement restartButton;
	restartButton.hasBackgroundColor = false;
	restartButton.path = "assets/textures/ui/standings/standings_restartgame.png";
	restartButton.textureID = GenerateTexture(restartButton.path.c_str(), false);
	restartButton.anchors = glm::vec4(0.3225, 0.833, 0.485, 0.9148);
	restartButton.anchorOffsets = glm::vec4(226, 0, 226, 0);
	// --- ---

	Entity e10 = coordinator->createEntity();
	coordinator->addComponent(e10, menuButton);

	Entity e11 = coordinator->createEntity();
	coordinator->addComponent(e11, restartButton);

	
	standingsScreen.UIElements.push_back(e10);
	standingsScreen.UIElements.push_back(e11);

	nameToScreen["standingsScreen"] = standingsScreen;
}

void UISystem::createRacingHUD() {
	UIElement menu1;
	// default anchors are whole screen (0,0,1,1)

	menu1.hasBackgroundColor = false; // render texture

	menu1.path = "assets/textures/ui/racing/racing_hud.png";
	menu1.textureID = GenerateTexture(menu1.path.c_str(), false);

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, menu1);

	UIScreen racingHUD;
	racingHUD.name = "racingHUD";
	racingHUD.UIElements.push_back(e1);

	nameToScreen["racingHUD"] = racingHUD;
}

void UISystem::createLapCounter() {
	UIElement lapc1;
	lapc1.text = "Lap: ";
	lapc1.textScale = 1.0f;
	// default anchors are whole screen (0,0,1,1)
	lapc1.textColor = glm::vec3(1.0f);
	lapc1.textAlignmentX = LEFT;
	lapc1.textAlignmentY = BOTTOM;

	lapc1.hasBackgroundColor = false;

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, lapc1);

	UIScreen lapCounter;
	lapCounter.name = "lapCounter";
	lapCounter.UIElements.push_back(e1);

	nameToScreen["lapCounter"] = lapCounter;
}

void UISystem::updateLapCounter(int lapCount) {
	// can assume it's only the first thing (we hard coded it above)
	Entity& e1 = nameToScreen["lapCounter"].UIElements[0];
	UIElement& u1 = coordinator->getComponent<UIElement>(e1);
	u1.text = "Lap: " + std::to_string(lapCount);
}