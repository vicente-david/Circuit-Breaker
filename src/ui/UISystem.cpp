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

// called by rendering likely
void UISystem::update() {
	// we will assume game automatically updates which UI elements to show
	// so loop through active UI elements

	// for all screens render the containers
	
	// optimization recalc matrix only on window resize
	textMat = glm::ortho(0.0f, static_cast<float>(*SCR_WIDTH), static_cast<float>(*SCR_HEIGHT), 0.0f);


	// render text
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1,
		GL_FALSE, glm::value_ptr(textMat));

	// for all screens render their text
	for (UIScreen screen : screenStack) {

		for (Entity& entity : screen.UIElements) {
			UIElement& uiElement = coordinator->getComponent<UIElement>(entity);

			if (!uiElement.text.empty()) {
				textPositions p1  = calculateTextContainer(uiElement);

				RenderText(textProg->id, textVAO, textVBO, uiElement.text, p1, 1.0f, glm::vec3(1.0f), textFont);
			}
			
			// additionally you can test for visiblity, if something else controls it
		}
	}
	
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
	uF.p1 = glm::vec3(left*width + leftO, bottom*height + bottomO, 0.0f); // (left, bottom, 0.0)
	uF.p2 = glm::vec3(right*width + rightO, bottom*height + bottomO, 0.0f); // (right, bottom, 0.0)
	uF.p3 = glm::vec3(right*width + rightO, top*height + topO, 0.0f); // (right, top, 0.0)

	uF.p4 = glm::vec3(left*width + leftO, bottom*height + bottomO, 0.0f); // (left, bottom, 0.0)
	uF.p5 = glm::vec3(right*width + rightO, top*height + topO, 0.0f); // (right, top, 0.0)
	uF.p6 = glm::vec3(left*width + leftO, top*height + topO, 0.0f); // (left, top, 0.0)
	
	return uF;
}

textPositions UISystem::calculateTextContainer(UIElement u1) {
	textPositions tp1;
	tp1.leftPx = u1.anchors.x*(*SCR_WIDTH) + u1.anchorOffsets.x;
	tp1.topPx = u1.anchors.y*(*SCR_HEIGHT) + u1.anchorOffsets.y;
	tp1.rightPx = u1.anchors.z*(*SCR_WIDTH) + u1.anchorOffsets.z;
	tp1.bottomPx = u1.anchors.w*(*SCR_HEIGHT) + u1.anchorOffsets.w;
	tp1.textAlignX = u1.textAlignmentX;
	tp1.textAlignY = u1.textAlignmentY;
	return tp1;
}

void UISystem::initializeRenderingParams(){
	textProg = std::make_unique<ShaderProgram>("shaders/testText.vert", "shaders/testText.frag");

	// ui initialization
	uiShader = std::make_unique <ShaderProgram>("shaders/ui.vert", "shaders/ui.frag"); // upd ui shader ptr

	uiMat = glm::ortho(0.0f, static_cast<float>(*SCR_WIDTH), 0.0f, static_cast<float>(*SCR_HEIGHT)); // create iniital ortho projection
	uiShader->use(); // use it first
	glUniformMatrix4fv(glGetUniformLocation(uiShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat)); // upload the uniform

	glGenVertexArrays(1, &uiVAO);
	glBindVertexArray(uiVAO);

	glGenBuffers(1, &uiVBO);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO);

	// modify as needed

	// currently 5 floats (x,y,r,g,b) per point
	// 6 total because that's how many you need for a quad (2 triangles)
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * 6, NULL, GL_DYNAMIC_DRAW);

	// first position is just (x,y)
	// position is mapped to layout = 0, size is 2 since (x,y) the stride is 5 because [x,y,r,g,b,x] skip 5 for the next starting point
	// and it starts at 0 so (void*)0
	// color is mapped to layout = 1 size is 3 since (r,g,b) the stride is 5 again [r,g,b,x,y,r] skips 5 to get to the next starting point
	// and it starts after 2 floats so (void*)(2*sizeof(float)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

	// enable both
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


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

	textFont = initFont("assets/SquareAntiqua-Book.ttf");
	textMat = glm::ortho(0.0f, static_cast<float>(*SCR_WIDTH), 0.0f,
		static_cast<float>(*SCR_HEIGHT));
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1,
		GL_FALSE, glm::value_ptr(textMat));

}

void UISystem::renderUI(){

	// optimization recalc matrix only on window resize
	textMat = glm::ortho(0.0f, static_cast<float>(*SCR_WIDTH), 0.0f,
		static_cast<float>(*SCR_HEIGHT));


	// render text
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1,
		GL_FALSE, glm::value_ptr(textMat));
	
	//RenderText(textProg->id, textVAO, textVBO, "FPS: " + fps, 10.0, SCR_HEIGHT - 50, 1.0f, glm::vec3(1.0f), textFont);


	// pretend this is the ui shader we're using
	//textProg->use();
	//RenderText(textProg->id, textVAO, textVBO, game.uiText.textContent, game.uiText.xPos, game.uiText.yPos, game.uiText.scale, game.uiText.col, textFont);

}

void UISystem::addScreen(std::string screenName) {
	screenStack.push_back(nameToScreen[screenName]);
}

void UISystem::screenInitialization(){
	createFPSCounter();
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
void UISystem::createFPSCounter(){
	
	UIElement counter1;
	counter1.text = "FPS: " + *fps;
	counter1.textScale = 1.0f;
	// default anchors are whole screen (0,0,1,1)
	counter1.textColor = glm::vec3(1.0f);
	counter1.textAlignmentX = RIGHT;
	counter1.textAlignmentY = BOTTOM;

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, counter1);

	UIScreen fpsCounter;
	fpsCounter.name = "fpsCounter";
	fpsCounter.UIElements.push_back(e1);

	nameToScreen["fpsCounter"] = fpsCounter;
}

void UISystem::updateFPSCounter() {
	// can assume it's only the first thing (we hard coded it above)
	Entity e1 = nameToScreen["fpsCounter"].UIElements[0];
	UIElement& u1 = coordinator->getComponent<UIElement>(e1);
	u1.text = "FPS: " + *fps;
}