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
void UISystem::update(std::string& fps) {
	// we will assume game automatically updates which UI elements to show
	// so loop through active UI elements
	
	// optimization recalc matrix only on window resize
	textMat = glm::ortho(0.0f, static_cast<float>(*SCR_WIDTH), 0.0f,
		static_cast<float>(*SCR_HEIGHT));


	// render text
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1,
		GL_FALSE, glm::value_ptr(textMat));

	RenderText(textProg->id, textVAO, textVBO, "FPS: " + fps, 10, *SCR_HEIGHT / 2, 1.0f, glm::vec3(1.0f), textFont);
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * 6, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

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

	textFont = initFont("assets/miamanueva.ttf");
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
