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
	resShader->use();
	glUniformMatrix4fv(glGetUniformLocation(resShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat));
	speedShader->use();
	glUniformMatrix4fv(glGetUniformLocation(speedShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat)); // upload the uniform
	slideShader->use();
	glUniformMatrix4fv(glGetUniformLocation(slideShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat)); // upload the uniform
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
		RenderText(textProg->id, textVAO, textVBO, u1.text, p1, u1.textScale, u1.textColor, textFont);
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
		RenderText(textProg->id, textVAO, textVBO, u1.text, p1, u1.textScale, u1.textColor, textFont);
	}

}

void UISystem::updateResBars(Entity& e) {
	// quick and dirty way
	resShader->use();

	glm::vec3 col;
	// update uniforms 
	glUniform1fv(glGetUniformLocation(resShader->id, "currentBoost"), 1, playerBoost);
	glUniform1fv(glGetUniformLocation(resShader->id, "maxBoost"), 1, maxPlayerBoost);
	glUniform1fv(glGetUniformLocation(resShader->id, "currentHealth"), 1, playerHealth);
	glUniform1fv(glGetUniformLocation(resShader->id, "maxHealth"), 1, maxPlayerHealth);
	col = glm::vec3(0.0f, 0.0f, 0.0f); // whatever throaway it's unused as of rn


	UIElement& u1 = coordinator->getComponent<UIElement>(e);
	glBindVertexArray(resVAO);
	glBindBuffer(GL_ARRAY_BUFFER, resVBO);
	resData.clear();

	// god awful for readibility, but we encode the texture coords as the xy comp of UIelement color
	// only reason we can do this is because it's hard coded that way
	// see how the screen creation for each of these
	UIPositions positions = calculateAnchorPositions(u1);
	// 6 points in a triangle based quad
	for (int i = 0; i < 6; i++) {
		UIResVertex v1;
		v1.position = positions.points[i];
		v1.uvCoord = glm::vec2(u1.colors[i].x, u1.colors[i].y);
		v1.resourceColor = col;
		resData.push_back(v1);
	}

	
	glUniform1fv(glGetUniformLocation(resShader->id, "isBoosting"), 1, &isBoosting);
		
	

	glBufferData(GL_ARRAY_BUFFER, resData.size() * 8 * sizeof(float), resData.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, resData.size());


}

void UISystem::updateSpeedometer(Entity& e) {
	// quick and dirty way
	speedShader->use();

	glm::vec3 col = glm::vec3(0.0);
	// update uniforms 
	
	// reuse res variables
	UIElement& u1 = coordinator->getComponent<UIElement>(e);
	glBindVertexArray(resVAO);
	glBindBuffer(GL_ARRAY_BUFFER, resVBO);
	resData.clear();

	// current angle update

	currentAngle += *playerSpeed/5.0f * dTime;
	currentAngle = fmod(currentAngle, glm::two_pi<float>()); // keep the angle between 0 and two pi 
	prevAngle = currentAngle - *playerSpeed * dTime * 20.0;

	// we want slight time based delay 

	// if the player is not boosting
	// if the player just start boosting
	// if the player has boosted for more than x amount of time, ripple the ui (this prevents spam ripple)
	if (*isPlayerBoosting) {
		isBoosting = 1.0f;
		timeBoosting += frameTime;
	}
	else {
		isBoosting = 0.0f;
		timeBoosting = 0.0;
	}

	glUniform1fv(glGetUniformLocation(speedShader->id, "currentAngle"), 1, &currentAngle);
	glUniform1fv(glGetUniformLocation(speedShader->id, "prevAngle"), 1, &prevAngle);
	
	// pass as uniform only if it's 0, or > 0.5
	if (timeBoosting == 0 || timeBoosting >= 0.25) {
		glUniform1fv(glGetUniformLocation(speedShader->id, "isBoosting"), 1, &isBoosting);
		glUniform1fv(glGetUniformLocation(speedShader->id, "timeBoosting"), 1, &timeBoosting);
	}


	// god awful for readibility, but we encode the texture coords as the xy comp of UIelement color
	// only reason we can do this is because it's hard coded that way
	// see how the screen creation for each of these
	UIPositions positions = calculateAnchorPositions(u1);
	// 6 points in a triangle based quad
	for (int i = 0; i < 6; i++) {
		UIResVertex v1;
		v1.position = positions.points[i];
		v1.uvCoord = glm::vec2(u1.colors[i].x, u1.colors[i].y);
		v1.resourceColor = glm::vec3(1.0f); // unused so doesn't matter
		resData.push_back(v1);
	}
	glBufferData(GL_ARRAY_BUFFER, resData.size() * 8 * sizeof(float), resData.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, resData.size());

	//prevAngle = currentAngle;

	
	textProg->use();
	textPositions p1 = calculateTextContainer(u1);
	RenderText(textProg->id, textVAO, textVBO, std::to_string((int)*playerSpeed)+" HZ", p1, u1.textScale, u1.textColor, textFont);
	
}

void UISystem::updateSlider(Entity& e) {
	// not the same shader but same layout
	slideShader->use();

	UIElement& u1 = coordinator->getComponent<UIElement>(e);
	resData.clear();
	glBindVertexArray(resVAO);
	glBindBuffer(GL_ARRAY_BUFFER, resVBO);

	
	// if it has a bacground color 
	// then push the position, and the color
	UIPositions positions = calculateAnchorPositions(u1);
	// 6 points in a triangle based quad
	for (int i = 0; i < 6; i++) {
		UIResVertex v1;
		v1.position = positions.points[i];
		v1.uvCoord = glm::vec2(u1.colors[i].x, u1.colors[i].y);
		//v1.hLightColor = coordinator->getComponent<Animatable>(e).hLightColor;
		resData.push_back(v1);
	}

	// currently unused so bandaid fix

	Animatable& a1 = coordinator->getComponent < Animatable>(e);

	if (a1.isEnabled) {
		glUniform1fv(glGetUniformLocation(slideShader->id, "maxVol"), 1, &masterMax);
		glUniform1fv(glGetUniformLocation(slideShader->id, "currentVol"), 1, &masterCur);
	}
	else {
		glUniform1fv(glGetUniformLocation(slideShader->id, "maxVol"), 1, &musicMax);
		glUniform1fv(glGetUniformLocation(slideShader->id, "currentVol"), 1, &musicCur);
	}
	
	if (a1.isSelected) {
		isHighBool = 1.0;
		
	}
	else {
		isHighBool = 0.0;
	}

	glUniform1fv(glGetUniformLocation(slideShader->id, "isHighlighted"), 1, &isHighBool);
	

	glBufferData(GL_ARRAY_BUFFER, resData.size() * 8 * sizeof(float), resData.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, resData.size());
	

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
	case(ANIM_BAR):
		updateResBars(e);
		break;
	case(ANIM_SPEEDOMETER):
		updateSpeedometer(e);
		break;
	case(ANIM_SLIDER):
		updateSlider(e);
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

	if (u1.aspectRatio <= 0.0) {
		return uF;
	}
	
	// otherwise do aspect ratio calculations

	// calculate center 
	glm::vec2 center = glm::vec2(0.5f*(uF.points[1].x + uF.points[0].x), 0.5f * (uF.points[2].y + uF.points[1].y));
	float boxWidth, boxHeight;
	// calculate height and width of box
	boxWidth = uF.points[1].x - uF.points[0].x;
	boxHeight = uF.points[1].y - uF.points[2].y; // our y is flipped so this becomes bottom-top

	float currentRatio = boxWidth / boxHeight; // current aspect ratio of the box

	float newWidth = boxWidth; // new height of the box
	float newHeight = boxHeight; // new width of the box

	// if the ratio is bigger, then shrink the dimensions
	if (currentRatio > u1.aspectRatio) {
		// shrink width
		// if the ratio is bigger that means the height is too small
		// or consequently the width is too big
		// targetW/targetH = x/currentHeight -> new width = aspectRatio*currentHeight;
		newWidth = u1.aspectRatio * boxHeight;

	}
	else {
		// shrink height
		// if the ratio is smaller, than that means the width is too small (the height is the denominator and it dominates)
		// or consequently the height is too big

		// targetW/targetH = currentWidth/x -> currentWidth*(targetH/targetW) = new Height
		newHeight = boxWidth / u1.aspectRatio;


	}

	// where the edge of the container resides
	// edge just means like for ex leftEdge = 3, means that
	// x=3, vertical line at x=3 (left edge means that x=3 is where the left edge is)
	float leftEdge, rightEdge, topEdge, bottomEdge; 
	

	// if the aspect ratio is off, then when shrinking the box to fit the aspect ratio
	// align center on horizontal
	if (u1.aRatioAlignX == CENTER) {
		// center align left/right
		leftEdge = center.x - 0.5f * newWidth;
		rightEdge = center.x + 0.5f * newWidth;
	}
	else if (u1.aRatioAlignX == LEFT) {
		// left align
		leftEdge = uF.points[0].x;
		rightEdge = leftEdge + newWidth;

	}
	else {
		//right align
		rightEdge = uF.points[1].x;
		leftEdge = rightEdge - newWidth;
			
	}

	// also align y if that is the case
	if (u1.aRatioAlignY == CENTER) {
		//center align top/bottom
		topEdge = center.y - 0.5f * newHeight;
		bottomEdge = center.y + 0.5f * newHeight;

	}
	else if (u1.aRatioAlignY == TOP) {
		// top align
		topEdge = uF.points[2].y;
		bottomEdge = topEdge + newHeight;
	}
	else {
		// bottom align
		bottomEdge = uF.points[0].y;
		topEdge = bottomEdge - newHeight;

	}

	uF.points.clear();

	uF.points.push_back(glm::vec3(leftEdge, bottomEdge, 0.0f));
	uF.points.push_back(glm::vec3(rightEdge, bottomEdge, 0.0f));
	uF.points.push_back(glm::vec3(rightEdge, topEdge, 0.0f));
	uF.points.push_back(glm::vec3(leftEdge, bottomEdge, 0.0f));
	uF.points.push_back(glm::vec3(rightEdge, topEdge, 0.0f));
	uF.points.push_back(glm::vec3(leftEdge, topEdge, 0.0f));

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

	// if it doesn't have an aspect ratio scale return
	if (u1.aspectRatio <= 0.0) {
		return tp1;
	}


	// otherwise do aspect ratio calculations

	// note tp1 is in screen space

	// calculate center 
	glm::vec2 center = glm::vec2(0.5f * (tp1.rightPx + tp1.leftPx), 0.5f * (tp1.bottomPx + tp1.topPx));
	float boxWidth, boxHeight;
	// calculate height and width of box
	boxWidth = tp1.rightPx - tp1.leftPx;
	boxHeight = tp1.bottomPx - tp1.topPx; // our y is flipped so this becomes bottom-top

	float currentRatio = boxWidth / boxHeight; // current aspect ratio of the box

	float newWidth = boxWidth; // new height of the box
	float newHeight = boxHeight; // new width of the box

	// if the ratio is bigger, then shrink the dimensions
	if (currentRatio > u1.aspectRatio) {
		// shrink width
		// if the ratio is bigger that means the height is too small
		// or consequently the width is too big
		// targetW/targetH = x/currentHeight -> new width = aspectRatio*currentHeight;
		newWidth = u1.aspectRatio * boxHeight;

	}
	else {
		// shrink height
		// if the ratio is smaller, than that means the width is too small (the height is the denominator and it dominates)
		// or consequently the height is too big

		// targetW/targetH = currentWidth/x -> currentWidth*(targetH/targetW) = new Height
		newHeight = boxWidth / u1.aspectRatio;


	}

	// where the edge of the container resides
	// edge just means like for ex leftEdge = 3, means that
	// x=3, vertical line at x=3 (left edge means that x=3 is where the left edge is)
	float leftEdge, rightEdge, topEdge, bottomEdge;


	// if the aspect ratio is off, then when shrinking the box to fit the aspect ratio
	// align center on horizontal
	if (u1.aRatioAlignX == CENTER) {
		// center align left/right
		leftEdge = center.x - 0.5f * newWidth;
		rightEdge = center.x + 0.5f * newWidth;
	}
	else if (u1.aRatioAlignX == LEFT) {
		// left align
		leftEdge = tp1.leftPx;
		rightEdge = leftEdge + newWidth;

	}
	else {
		//right align
		rightEdge = tp1.rightPx;
		leftEdge = rightEdge - newWidth;

	}

	// also align y if that is the case
	if (u1.aRatioAlignY == CENTER) {
		//center align top/bottom
		topEdge = center.y - 0.5f * newHeight;
		bottomEdge = center.y + 0.5f * newHeight;

	}
	else if (u1.aRatioAlignY == TOP) {
		// top align
		topEdge = tp1.topPx;
		bottomEdge = topEdge + newHeight;
	}
	else {
		// bottom align
		bottomEdge = tp1.bottomPx;
		topEdge = bottomEdge - newHeight;

	}

	tp1.leftPx = leftEdge;
	tp1.topPx = topEdge;
	tp1.rightPx = rightEdge;
	tp1.bottomPx = bottomEdge;



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

	//textFont = initFont("assets/PixelifySans-VariableFont_wght.ttf");
	textFont = initFont("assets/charles.ttf");
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


	// resource bar (health/boost)
	resShader = std::make_unique<ShaderProgram>("shaders/bar.vert", "shaders/bar.frag");

	resShader->use();

	glUniformMatrix4fv(glGetUniformLocation(resShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat)); // upload the uniform
	glGenVertexArrays(1, &resVAO);
	glBindVertexArray(resVAO);

	glGenBuffers(1, &resVBO);
	glBindBuffer(GL_ARRAY_BUFFER, resVBO);


	// 8 floats, 6 things for a quad
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 6, NULL, GL_DYNAMIC_DRAW);

	// position, uv, color
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));


	// enable all
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// layout, vec3, vec2, vec3

	// we will reuse res vao and vbo for speed
	speedShader = std::make_unique<ShaderProgram>("shaders/speeeeeed.vert", "shaders/speeeeeed.frag");

	speedShader->use();

	glUniformMatrix4fv(glGetUniformLocation(speedShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat)); // upload the uniform


	// we will reuse res vao and vbo for speed
	slideShader = std::make_unique<ShaderProgram>("shaders/slider.vert", "shaders/slider.frag");

	slideShader->use();

	glUniformMatrix4fv(glGetUniformLocation(slideShader->id, "projection"), 1, GL_FALSE, glm::value_ptr(uiMat)); // upload the uniform

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
	int animatableElements = 0;
	for (Entity& entity : screenStack.back().UIElements) {
		if (coordinator->hasComponent<Animatable>(entity))
			animatableElements++;
	}
	return animatableElements;
	//int totalElements = (int)screenStack.back().UIElements.size();
	//return std::max(0, totalElements - 1); // subtract background
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
	createCountdown();
	createBackwardsDisplay();
	createResourceBar();
	createSpeedometer();
	createControlsMenu();
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
	counter1.textScale = 0.5f;
	// default anchors are whole screen (0,0,1,1)
	counter1.anchorOffsets = glm::vec4(0.0f, 15.0f, -15.0f, 0.0f);
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
	counter1.text = "POSITION: " + std::to_string(0);
	counter1.textScale = 0.75f;
	// default anchors are whole screen (0,0,1,1)
	counter1.anchorOffsets = glm::vec4(0.0f, 0.0f, -15.0f, -15.0f);
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

	u1.text = "POSITION: " + std::to_string(placement+1);
}

void UISystem::createMainMenu() {
	UIElement menu1;
	//menu1.text = "WOAH THE MENU CAN HAVE INDEPENDENT TEXT CRAZY!";
	//menu1.textScale = 1.0f;
	// default anchors are whole screen (0,0,1,1)
	//menu1.textColor = glm::vec3(1.0f);
	//menu1.textAlignmentX = CENTER;
	//menu1.textAlignmentY = BOTTOM;

	UIElement menuBorder; // used for painting the background of the main menu (after aspect correction)
	menuBorder.hasBackgroundColor = true; // enable bg color
	// default anchors are full screen
	for (int i = 0; i < 6; i++) {
		menuBorder.colors[i] = glm::vec3(18.0f/255.0f, 28.0f/255.0f, 24.0f/255.0);
	}
	Entity bgMenuColor = coordinator->createEntity();
	coordinator->addComponent(bgMenuColor, menuBorder);
	menu1.aspectRatio = 1101.0 / 786.0;

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

	startGameButton.aspectRatio = 450.0f / 64.0f; // based on the figma design
	
	// exit button 
	UIElement exitButton;
	exitButton.hasBackgroundColor = false;
	exitButton.path = "assets/textures/ui/mainMenu/startmenu_exitgame.png";
	exitButton.textureID = GenerateTexture(exitButton.path.c_str(), false);
	exitButton.anchors = glm::vec4(0.3, 0.8525, 0.7, 0.9325);
	exitButton.aspectRatio = 450.0f / 64.0f; // based on the figma design

	// settings button
	UIElement settingsButton;
	settingsButton.hasBackgroundColor = false;
	settingsButton.path = "assets/textures/ui/mainMenu/startmenu_settings.png";
	settingsButton.textureID = GenerateTexture(settingsButton.path.c_str(), false);

	settingsButton.anchors = glm::vec4(0.3, 0.66539, 0.7, 0.74682);
	settingsButton.anchorOffsets = glm::vec4(0, -12, 0, -12);
	settingsButton.aspectRatio = 450.0f / 64.0f; // based on the figma design

	UIElement tutButton;
	tutButton.hasBackgroundColor = false;
	tutButton.path = "assets/textures/ui/mainMenu/startmenu_controls.png";
	tutButton.textureID = GenerateTexture(tutButton.path.c_str(), false);

	tutButton.anchors = glm::vec4(0.3, 0.7525, 0.7, 0.8325);
	tutButton.aspectRatio = 450.0f / 64.0f; // based on the figma design

	// --- ---

	Entity e2 = coordinator->createEntity();
	coordinator->addComponent(e2, startGameButton); // button 0
	coordinator->addComponent(e2, Animatable());

	Entity e3 = coordinator->createEntity();
	coordinator->addComponent(e3, settingsButton); // button 1
	coordinator->addComponent(e3, Animatable());

	Entity e5 = coordinator->createEntity();
	coordinator->addComponent(e5, tutButton); // button 1
	coordinator->addComponent(e5, Animatable());

	Entity e4 = coordinator->createEntity();
	coordinator->addComponent(e4, exitButton); // button 2
	coordinator->addComponent(e4, Animatable());

	UIScreen mainMenu;
	mainMenu.UIElements.push_back(bgMenuColor);
	mainMenu.name = "mainMenu";
	mainMenu.UIElements.push_back(e1);
	
	mainMenu.UIElements.push_back(e2);
	mainMenu.UIElements.push_back(e3);
	mainMenu.UIElements.push_back(e5);
	mainMenu.UIElements.push_back(e4);

	nameToScreen["mainMenu"] = mainMenu;
}

void UISystem::createPauseMenu() {
	UIElement menu1;
	// default anchors are whole screen (0,0,1,1)

	menu1.hasBackgroundColor = false; // render texture

	menu1.path = "assets/textures/ui/pauseMenu/pause_bg.png";
	menu1.textureID = GenerateTexture(menu1.path.c_str(), false);
	
	UIElement menuBorder; // used for painting the background of the main menu (after aspect correction)
	menuBorder.hasBackgroundColor = true; // enable bg color
	// default anchors are full screen
	for (int i = 0; i < 6; i++) {
		menuBorder.colors[i] = glm::vec3(18.0f / 255.0f, 28.0f / 255.0f, 24.0f / 255.0);
	}
	Entity bgMenuColor = coordinator->createEntity();
	coordinator->addComponent(bgMenuColor, menuBorder);
	menu1.aspectRatio = 1101.0 / 786.0;


	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, menu1);

	// --- BUTTONS --

	// resume button
	UIElement resumeButton;
	resumeButton.hasBackgroundColor = false;
	resumeButton.path = "assets/textures/ui/pauseMenu/pause_resume.png";
	resumeButton.textureID = GenerateTexture(resumeButton.path.c_str(), false);
	resumeButton.anchors = glm::vec4(0.3, 0.4528, 0.7, 0.5343);
	resumeButton.aspectRatio = 450.0f / 64.0f; // based on the figma design

	// settings button
	UIElement settingsButton;
	settingsButton.hasBackgroundColor = false;
	settingsButton.path = "assets/textures/ui/pauseMenu/pause_settings.png";
	settingsButton.textureID = GenerateTexture(settingsButton.path.c_str(), false);
	settingsButton.anchors = glm::vec4(0.3, 0.5343+0.06, 0.7, 0.6158+0.06); // 0.06 is just 48/800 (our default height and divided by the anchor offset calculated below)
	//settingsButton.anchorOffsets = glm::vec4(0, 48, 0, 48);
	settingsButton.aspectRatio = 450.0f / 64.0f; // based on the figma design

	// exit button 
	UIElement exitButton;
	exitButton.hasBackgroundColor = false;
	exitButton.path = "assets/textures/ui/pauseMenu/pause_quittomenu.png";
	exitButton.textureID = GenerateTexture(exitButton.path.c_str(), false);
	exitButton.anchors = glm::vec4(0.3, 0.6158+0.06*2, 0.7, 0.6973+0.06*2);
	//exitButton.anchorOffsets = glm::vec4(0, 96, 0, 96);
	exitButton.aspectRatio = 450.0f / 64.0f; // based on the figma design

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
	pauseMenu.UIElements.push_back(bgMenuColor);
	pauseMenu.UIElements.push_back(e1);

	pauseMenu.UIElements.push_back(e2);
	pauseMenu.UIElements.push_back(e3);
	pauseMenu.UIElements.push_back(e4);

	nameToScreen["pauseMenu"] = pauseMenu;
}
void UISystem::createControlsMenu() {

	UIElement menuBorder; // used for painting the background of the main menu (after aspect correction)
	menuBorder.hasBackgroundColor = true; // enable bg color
	// default anchors are full screen
	for (int i = 0; i < 6; i++) {
		menuBorder.colors[i] = glm::vec3(19.0f / 255.0f, 38.0f / 255.0f, 30.0f / 255.0);
	}
	Entity bgMenuColor = coordinator->createEntity();
	coordinator->addComponent(bgMenuColor, menuBorder);

	UIElement transitionText;
	transitionText.text = "a NEXT";
	transitionText.textAlignmentX = RIGHT;
	transitionText.textAlignmentY = BOTTOM;
	transitionText.textScale = 1.0f;
	transitionText.textColor = glm::vec3(1.0f);
	transitionText.hasBackgroundColor = false;
	transitionText.aspectRatio = 3840.0 / 2160.0;
	transitionText.anchors = glm::vec4(0.90, 0.90, 1.0, 1.0);
	transitionText.anchorOffsets = glm::vec4(0.0, 0.0, -5.0, 0.0);

	UIElement tutMenu;
	tutMenu.path = "assets/textures/ui/tutorial/tutorial.png";
	tutMenu.hasBackgroundColor = false;
	tutMenu.textureID = GenerateTexture(tutMenu.path.c_str(), false);
	tutMenu.aspectRatio = 3840.0 / 2160.0;

	UIElement controlMenu;
	controlMenu.path = "assets/textures/ui/tutorial/controls.png";
	controlMenu.hasBackgroundColor = false;
	controlMenu.textureID = GenerateTexture(controlMenu.path.c_str(), false);
	controlMenu.aspectRatio = 3840.0 / 2160.0;

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, tutMenu);

	Entity e2 = coordinator->createEntity();
	coordinator->addComponent(e2, controlMenu);

	Entity textTrans = coordinator->createEntity();
	coordinator->addComponent(textTrans, transitionText);

	UIElement button;
	button.anchors = glm::vec4(1,1,0,0);

	UIScreen menu1;
	menu1.name = "tutorial";
	menu1.UIElements.push_back(bgMenuColor);
	menu1.UIElements.push_back(e1);
	menu1.UIElements.push_back(textTrans);

	UIScreen menu2;
	menu2.name = "controls";
	menu2.UIElements.push_back(bgMenuColor);
	menu2.UIElements.push_back(e2);
	menu2.UIElements.push_back(textTrans);

	nameToScreen["tutorial"] = menu1;
	nameToScreen["controls"] = menu2;
}

void UISystem::createSettingsMenu() {
	UIElement menu1;
	// default anchors are whole screen (0,0,1,1)

	menu1.hasBackgroundColor = false; // render texture

	menu1.path = "assets/textures/ui/settings/settings_bg.png";
	menu1.textureID = GenerateTexture(menu1.path.c_str(), false);

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, menu1);

	// sliders
	UIElement s1;
	s1.hasBackgroundColor = false;
	s1.anchors = glm::vec4(0.11625, 0.425, 0.5-0.01, 0.45);
	s1.aspectRatio = 414.0f / 8.0f;

	UIElement s2;
	s2.hasBackgroundColor = false;
	s2.anchors = glm::vec4(0.5+0.01, 0.425, 1.0-0.11625, 0.45);
	s2.aspectRatio = 414.0f / 8.0f;

	// --- BUTTONS ---

	// easy button
	UIElement b1;
	b1.hasBackgroundColor = false;
	b1.path = "assets/textures/ui/settings/settings_fps.png";
	b1.textureID = GenerateTexture(b1.path.c_str(), false);
	b1.anchors = glm::vec4(0.11625, 0.625, 0.5-0.01, 1.0);
	b1.aspectRatio = 416.0f / 40.0f;
	b1.aRatioAlignY = TOP;

	UIElement b2;
	b2.hasBackgroundColor = false;
	b2.path = "assets/textures/ui/settings/settings_speedometer.png";
	b2.textureID = GenerateTexture(b2.path.c_str(), false);
	b2.anchors = glm::vec4(0.5+0.01, 0.625, 1.0 - 0.11625, 1.0);
	b2.aspectRatio = 416.0f / 40.0f;
	b2.aRatioAlignY = TOP;

	UIElement b3;
	b3.hasBackgroundColor = false;
	b3.path = "assets/textures/ui/settings/settings_backtomenu.png";
	b3.textureID = GenerateTexture(b3.path.c_str(), false);
	b3.anchors = glm::vec4(0.093, 0.75, 1.0-0.093, 1.0);
	b3.aspectRatio = 896.0 / 64.0f;
	b3.aRatioAlignY = TOP;
	

	// --- ---

	Entity e2 = coordinator->createEntity();
	coordinator->addComponent(e2, b1); // button 0
	coordinator->addComponent(e2, Animatable());

	Entity e3 = coordinator->createEntity();
	coordinator->addComponent(e3, b2); // button 1
	coordinator->addComponent(e3, Animatable());

	Entity e6 = coordinator->createEntity();
	coordinator->addComponent(e6, b3); // button 1
	coordinator->addComponent(e6, Animatable());

	// slider

	Entity e4 = coordinator->createEntity();
	coordinator->addComponent(e4, s1); // button 2
	coordinator->addComponent(e4, Animatable{true, false, ANIM_SLIDER});

	Entity e5 = coordinator->createEntity();
	coordinator->addComponent(e5, s2); // button 3
	coordinator->addComponent(e5, Animatable{false, false, ANIM_SLIDER });



	UIScreen settingsMenu;
	settingsMenu.name = "settingsMenu";
	settingsMenu.UIElements.push_back(e1);

	// slider
	settingsMenu.UIElements.push_back(e4);
	settingsMenu.UIElements.push_back(e5);

	// buttons
	settingsMenu.UIElements.push_back(e2);
	settingsMenu.UIElements.push_back(e3);
	settingsMenu.UIElements.push_back(e6);
	

	nameToScreen["settingsMenu"] = settingsMenu;
}

void UISystem::createStandingsScreen(Leaderboard& lb) {
	UIElement menu1;
	// default anchors are whole screen (0,0,1,1)

	menu1.hasBackgroundColor = false; // render texture

	menu1.path = "assets/textures/ui/standings/standings_bg.png";
	menu1.textureID = GenerateTexture(menu1.path.c_str(), false);
	
	UIElement menuBorder; // used for painting the background of the main menu (after aspect correction)
	menuBorder.hasBackgroundColor = true; // enable bg color
	// default anchors are full screen
	for (int i = 0; i < 6; i++) {
		menuBorder.colors[i] = glm::vec3(18.0f / 255.0f, 28.0f / 255.0f, 24.0f / 255.0);
	}
	menu1.aspectRatio = 1101.0 / 786.0;

	Entity bgMenuColor = coordinator->createEntity();
	coordinator->addComponent(bgMenuColor, menuBorder);

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, menu1);

	UIScreen standingsScreen;
	standingsScreen.name = "standingsScreen";
	standingsScreen.UIElements.push_back(bgMenuColor);
	standingsScreen.UIElements.push_back(e1);

	

	// --- POSITIONS FIELDS ---
	for (int i = 0; i < lb.standings.size(); i++) {
		UIElement firstPlace;
		firstPlace.hasBackgroundColor = false;
		firstPlace.path = "assets/textures/ui/standings/standings_position.png";
		firstPlace.textureID = GenerateTexture(firstPlace.path.c_str(), false);
		firstPlace.anchors = glm::vec4(0.3225, 0.145+0.0825*i, 0.6785, 0.2252+ 0.0825*i); // 0.0825 vertical offset of 66 pixels for 800 height
		firstPlace.aspectRatio = 450.0 / 64.0;
		//firstPlace.anchorOffsets = glm::vec4(0, 66*i, 0, 66*i);
		firstPlace.text = " " + std::to_string(i+1) + ". "+lb.standings[i];
		firstPlace.textScale = 0.75f;
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
	menuButton.anchors = glm::vec4(0.3225-0.01, 0.833, 0.5-0.01, 0.9148);
	menuButton.aspectRatio = 176 / 64.0;
	menuButton.aRatioAlignX = RIGHT;
	

	// restart game button
	UIElement restartButton;
	restartButton.hasBackgroundColor = false;
	restartButton.path = "assets/textures/ui/standings/standings_restartgame.png";
	restartButton.textureID = GenerateTexture(restartButton.path.c_str(), false);
	restartButton.anchors = glm::vec4(0.5+0.01, 0.833, 0.6785+0.01, 0.9148);
	restartButton.aspectRatio = 176 / 64.0;
	restartButton.aRatioAlignX = LEFT;
	// --- ---

	Entity e10 = coordinator->createEntity();
	coordinator->addComponent(e10, menuButton);
	coordinator->addComponent(e10, Animatable());

	Entity e11 = coordinator->createEntity();
	coordinator->addComponent(e11, restartButton);
	coordinator->addComponent(e11, Animatable());

	
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
	lapc1.text = "LAP: 1";
	lapc1.textScale = 0.75f;
	// default anchors are whole screen (0,0,1,1)
	lapc1.anchorOffsets = glm::vec4(15.0f, 0.0f, 0.0f, -15.0f);
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
	u1.text = "LAP: " + std::to_string(lapCount);
}

void UISystem::createCountdown() {
	UIElement counter1;
	counter1.text = "";
	counter1.textScale = 5.0f;
	// default anchors are whole screen (0,0,1,1)
	counter1.textColor = glm::vec3(1.0f);
	counter1.textAlignmentX = CENTER;
	counter1.textAlignmentY = CENTER;

	counter1.hasBackgroundColor = false;

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, counter1);

	UIScreen countDown;
	countDown.name = "countDown";
	countDown.UIElements.push_back(e1);

	nameToScreen["countDown"] = countDown;
}

void UISystem::updateCountdown(std::string second, float time) {
	// can assume it's only the first thing (we hard coded it above)
	Entity& e1 = nameToScreen["countDown"].UIElements[0];
	UIElement& u1 = coordinator->getComponent<UIElement>(e1);
	u1.text = second;
}

void UISystem::createBackwardsDisplay() {
	UIElement counter1;
	counter1.text = "";
	counter1.textScale = 3.0f;
	// default anchors are whole screen (0,0,1,1)
	counter1.textColor = glm::vec3(1.0f);
	counter1.textAlignmentX = CENTER;
	counter1.textAlignmentY = CENTER;

	counter1.hasBackgroundColor = false;

	Entity e1 = coordinator->createEntity();
	coordinator->addComponent(e1, counter1);

	UIScreen backwardsDisplay;
	backwardsDisplay.name = "backwardsDisplay";
	backwardsDisplay.UIElements.push_back(e1);

	nameToScreen["backwardsDisplay"] = backwardsDisplay;
}

void UISystem::updateBackwardsDisplay(float time) {
	// if player is backwards then display "BACKWARDS" for 1 second
	// off for one second, and then back on again
	Entity& e1 = nameToScreen["backwardsDisplay"].UIElements[0];
	UIElement& u1 = coordinator->getComponent<UIElement>(e1);

	if (*playerBackwards) {
		// update the clock
		backwardClock.update(time);

		
		// if the timer has completed, restart
		if (backwardClock.completedTimer()) {
			backwardClock.start(2.0);
		}

		if ((backwardClock.timerDuration - backwardClock.remaining) < 1.0) {
			// show backwards
			u1.text = "BACKWARDS!!!";
		}
		else {
			// show nothing
			u1.text = "";
		}
		
	}
	else {
		// reset if player isn't backwards
		backwardClock.resetTimer();
		u1.text = "";
	}


}


void UISystem::createResourceBar() {
	UIElement hBar;
	// default anchors are whole screen (0,0,1,1)
	hBar.anchors = glm::vec4(0.0, 0.0, 0.35, 0.0);
	hBar.anchorOffsets = glm::vec4(10.0, 10.0, 0.0, 0.0);
	hBar.hasBackgroundColor = false;

	hBar.aspectRatio = 12.0f / 1.0f; // for every 10 width, 1 height
	hBar.aRatioAlignX = LEFT; // left align it 
	hBar.aRatioAlignY = TOP;

	Entity e1 = coordinator->createEntity();

	Animatable a = { true, false, ANIM_BAR};
	coordinator->addComponent(e1, hBar);
	coordinator->addComponent(e1, a);

	UIScreen myHealthIsDeclining;
	myHealthIsDeclining.name = "myHealthIsDeclining";
	myHealthIsDeclining.UIElements.push_back(e1);

	nameToScreen["myHealthIsDeclining"] = myHealthIsDeclining;
}

void UISystem::createSpeedometer() {
	UIElement speedometer;
	// default anchors are whole screen (0,0,1,1)
	speedometer.anchors = glm::vec4(0.75, 0.5, 1.0, 1.0);
	speedometer.anchorOffsets = glm::vec4(0.0, 0.0, 0.0, -64.0);
	speedometer.hasBackgroundColor = false;
	speedometer.text = "0";
	speedometer.textScale = 0.75f;
	speedometer.textColor = glm::vec3(1.0f);
	speedometer.textAlignmentY = CENTER;
	speedometer.textAlignmentX = CENTER;

	speedometer.aspectRatio = 1.0f;
	speedometer.aRatioAlignX = RIGHT;
	speedometer.aRatioAlignY = BOTTOM;
	

	Entity e1 = coordinator->createEntity();

	Animatable a = { true, false, ANIM_SPEEDOMETER };
	coordinator->addComponent(e1, speedometer);
	coordinator->addComponent(e1, a);

	UIScreen speeeeeed;
	speeeeeed.name = "speeeeeed";
	speeeeeed.UIElements.push_back(e1);


	nameToScreen["speeeeeed"] = speeeeeed;
}
