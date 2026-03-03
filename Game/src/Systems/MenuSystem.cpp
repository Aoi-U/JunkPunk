#include "MenuSystem.h"

#include "../ECSController.h"

extern ECSController controller;

MenuSystem::MenuSystem()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	textShader = std::make_unique<Shader>("assets/shaders/text.vert", "assets/shaders/text.frag");
	uiShader = std::make_unique<Shader>("assets/shaders/ui.vert", "assets/shaders/ui.frag");

	controller.AddEventListener(Events::Window::RESIZED, [this](Event& e) {this->WindowSizeListener(e); });
	controller.AddEventListener(Events::Window::INPUT, [this](Event& e) { this->KeyboardInputListener(e); });
}

void MenuSystem::Init(std::shared_ptr<Gamepad> gamepad)
{	
	this->gamepad = gamepad;

	fonts = Text();
	textVAO = VAO();
	textVBO = VBO();
	fonts.initVAO(&textVAO, &textVBO);
	fonts.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));

	textShader->use();
	textShader->setMat4("u_projection", fonts.projMat);

	uiVAO = VAO();
	uiVBO = VBO();
	uiVAO.Bind();
	uiVBO.Bind();

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	uiVAO.Unbind();
	uiVBO.Unbind();

	uiShader->use();
	uiShader->setMat4("u_projection", fonts.projMat);
	uiShader->setInt("u_texture", 0);

	InitUI();
	InitControlsUI();
	InitEndUI();
}

void MenuSystem::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MenuSystem::Update()
{
	if (gamepad->LStick_InDeadzone())
	{
		canNavigate = true;
	}
	
	if (canNavigate)
	{
		if (gamepad->LeftStick_Y() > 0.5f) // navigate left
		{
			// navigate left
			if (currentHover > 0)
			{
				Event event(Events::Audio::PLAY_SOUND);
				event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
				event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
				event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
				controller.SendEvent(event);

				currentHover--;
			}
			canNavigate = false;
		}
		else if (gamepad->LeftStick_Y() < -0.5f) // navigate right
		{
			if (currentHover < 2)
			{
				Event event(Events::Audio::PLAY_SOUND);
				event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
				event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
				event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
				controller.SendEvent(event); 

				currentHover++;
			}
			canNavigate = false;
		}
	}

	if (gamepad->GetButtonDown(Buttons::JUMP))
	{
		switch (currentHover)
		{
		case Menus::START:
		{
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::CONTROLS);
			controller.SendEvent(event);
			break;
		}
		case Menus::SETTINGS:
		{
			// settings menu
			break;
		}
		case Menus::QUIT:
		{
			// quit game
			controller.SendEvent(Events::Window::CLOSE);
			break;
		}
		}
	}



	glDisable(GL_DEPTH_TEST);
	Clear(0.1f, 0.1f, 0.1f, 1.0f);

	RenderElements(uiElements);

	// Render text with scaled positions and uniform scale for text size
	float buttonSpacing = 120.0f;
	float textScale = uniformScale;
	RenderText("START", ScaledX(550.0f), ScaledY(480.0f), textScale,
		currentHover == Menus::START ? hoverColor : defaultColor);
	RenderText("SETTINGS", ScaledX(510.0f), ScaledY(480.0f - buttonSpacing), textScale,
		currentHover == Menus::SETTINGS ? hoverColor : defaultColor);
	RenderText("QUIT", ScaledX(570.0f), ScaledY(480.0f - buttonSpacing * 2), textScale,
		currentHover == Menus::QUIT ? hoverColor : defaultColor);

}

void MenuSystem::Reset()
{
	canNavigate = true;
	currentHover = 0;
}

void MenuSystem::RenderWinText() {
	if (!playerWon)
		return;
	glDisable(GL_DEPTH_TEST);
	float scale = uniformScale * 2.0f;
	RenderText(
		"YOU WIN!",
		ScaledX(500.0f),
		ScaledY(600.0f),
		scale,
		glm::vec3(0.2f, 1.0f, 0.2f)
	);
}

void MenuSystem::RenderEndScreen() {
	glDisable(GL_DEPTH_TEST);
	Clear(0.05f, 0.05f, 0.05f, 1.0f);
	float titleScale = uniformScale * 2.0f;

	RenderElements(endUIElements);

	RenderText(
		"YOU WIN!",
		ScaledX(480.0f),
		ScaledY(500.0f),
		titleScale,
		glm::vec3(0.2f, 1.0f, 0.2f)
	);

	RenderText(
		"Press A to return to Main Menu",
		ScaledX(380.0f),
		ScaledY(150.0f),
		uniformScale,
		glm::vec3(1.0f)
	);

	if (gamepad->GetButtonDown(Buttons::JUMP))
	{
		playerWon = false;
		Event event(Events::GameState::NEW_STATE);
		event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::STARTMENU);
		controller.SendEvent(event);
	}
}

void MenuSystem::RenderControlsScreen() {
	glDisable(GL_DEPTH_TEST);
	Clear(0.05f, 0.05f, 0.05f, 1.0f);
	RenderElements(controlsUIElements);

	if (gamepad->GetButtonDown(Buttons::JUMP)) {
		Event event(Events::GameState::NEW_STATE);
		event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::GAME);
		controller.SendEvent(event);
	}
}

void MenuSystem::RenderFadeOverlay(float alpha) {
	if (alpha <= 0.0f)
		return;

	glDisable(GL_DEPTH_TEST);

	RenderUIRect(
		0.0f,
		0.0f,
		(float)screenWidth,
		(float)screenHeight,
		glm::vec4(0.0f, 0.0f, 0.0f, alpha)
	);
}

void MenuSystem::RenderUIRect(float x, float y, float width, float height, glm::vec4 color)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	uiShader->use();
	uiShader->setVec4("u_color", &color.x);
	uiShader->setInt("u_useTexture", 0);
	uiShader->setMat4("u_projection", fonts.projMat);

	float vertices[6][4] =
	{
		{ x, y + height, 0.0f, 0.0f },
		{ x, y, 0.0f, 1.0f },
		{ x + width, y, 1.0f, 1.0f },

		{ x, y + height, 0.0f, 0.0f },
		{x + width, y, 1.0f, 1.0f },
		{x + width, y + height, 1.0f, 0.0f},
	};

	uiVAO.Bind();
	uiVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	uiVAO.Unbind();
}

void MenuSystem::RenderUITexture(float x, float y, float width, float height, Texture* tex, glm::vec4 tint)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	uiShader->use();
	uiShader->setVec4("u_color", &tint.x);
	uiShader->setInt("u_useTexture", 1);
	uiShader->setMat4("u_projection", fonts.projMat);

	float vertices[6][4] = {
		{ x,         y + height, 0.0f, 0.0f },
		{ x,         y,          0.0f, 1.0f },
		{ x + width, y,          1.0f, 1.0f },

		{ x,         y + height, 0.0f, 0.0f },
		{ x + width, y,          1.0f, 1.0f },
		{ x + width, y + height, 1.0f, 0.0f }
	};

	tex->Bind(GL_TEXTURE0);

	uiVAO.Bind();
	uiVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	uiVAO.Unbind();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void MenuSystem::InitUI()
{
	uiElements.clear();
	Texture tex("background.png");	
	tex.Load("assets/textures");
	uiElements.emplace_back(0.0f, 0.0f, 1280.0f, 720.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), ScaleMode::FILL, std::make_unique<Texture>(tex));


	float buttonWidth = 300.0f;
	float buttonHeight = 80.0f;
	float buttonX = 640.0f - buttonWidth * 0.5f;
	float buttonSpacing = 120.0f;

	float startY = 500.0f - buttonHeight * 0.5f;
	float settingsY = startY - buttonSpacing;
	float quitY = settingsY - buttonSpacing;

	// Create UI elements for each button
	uiElements.emplace_back(buttonX, startY, buttonWidth, buttonHeight,
		glm::vec4(0.2f, 0.2f, 0.2f, 0.9f));
	uiElements.emplace_back(buttonX, settingsY, buttonWidth, buttonHeight,
		glm::vec4(0.2f, 0.2f, 0.2f, 0.9f));
	uiElements.emplace_back(buttonX, quitY, buttonWidth, buttonHeight,
		glm::vec4(0.2f, 0.2f, 0.2f, 0.9f));
}

void MenuSystem::InitEndUI() {
	endUIElements.clear();
	Texture tex("dumpster sunset.jpg");
	tex.Load("assets/textures");
	endUIElements.emplace_back(
		0.0f,
		0.0f,
		1280.0f,
		720.0f,
		glm::vec4(1.0f),
		ScaleMode::FILL,
		std::make_unique<Texture>(tex)
	);
}

void MenuSystem::InitControlsUI() {
	controlsUIElements.clear();
	Texture tex("Powerup.png");
	tex.Load("assets/textures");

	controlsUIElements.emplace_back(
		0.0f,
		0.0f,
		1280.0f,
		720.0f,
		glm::vec4(1.0f),
		ScaleMode::FILL,
		std::make_unique<Texture>(tex)
	);
}

void MenuSystem::RenderElements(std::vector<UIElement>& elements)
{
	for (size_t i = 0; i < elements.size(); i++)
	{
		UIElement& elem = elements[i];

		glm::vec2 scaledPos{};
		glm::vec2 scaledSize{};

		switch (elem.scaleMode)
		{
		case ScaleMode::FIT:
		{
			scaledPos = ScaledPosition(elem.x, elem.y);
			scaledSize = ScaledSize(elem.width, elem.height);
			break;
		}
		case ScaleMode::FILL:
		{
			float screenAspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
			float elemAspect = elem.width / elem.height;

			float fillScale;
			float fillOffsetX = 0.0f;
			float fillOffsetY = 0.0f;

			if (screenAspect > elemAspect)
			{
				fillScale = static_cast<float>(screenWidth) / elem.width;
				fillOffsetY = (screenHeight - elem.height * fillScale) * 0.5f;
			}
			else
			{
				fillScale = static_cast<float>(screenHeight) / elem.height;
				fillOffsetX = (screenWidth - elem.width * fillScale) * 0.5f;
			}

			scaledPos = glm::vec2(elem.x * fillScale + fillOffsetX, elem.y * fillScale + fillOffsetY);
			scaledSize = glm::vec2(elem.width * fillScale, elem.height * fillScale);
			break;
		}
		case ScaleMode::STRETCH:
		{
			scaledPos = glm::vec2((elem.x / 1280.0f) * screenWidth, (elem.y / 720.0f) * screenHeight);
			scaledSize = glm::vec2((elem.width / 1280.0f) * screenWidth, (elem.height / 720.0f) * screenHeight);
			break;
		}
		}


		if (elem.tex)
		{
			RenderUITexture(scaledPos.x, scaledPos.y, scaledSize.x, scaledSize.y, elem.tex.get(), elem.color);
		}
		else
		{
			RenderUIRect(scaledPos.x, scaledPos.y, scaledSize.x, scaledSize.y, elem.color);
		}
	}
}

void MenuSystem::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	textShader->use();
	textShader->setVec3("textColor", &color.x);
	textShader->setMat4("u_projection", fonts.projMat);
	glActiveTexture(GL_TEXTURE0);
	textVAO.Bind();

	// iterate through all characters
	std::string::const_iterator c;

	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = fonts.charArial[*c];
		float xPos = x + ch.bearing.x * scale;
		float yPos = y - (ch.size.y - ch.bearing.y) * scale;

		float w = ch.size.x * scale;
		float h = ch.size.y * scale;

		// update vbo for each character
		float vertices[6][4] =
		{
			{xPos,			yPos + h, 0.0f, 0.0f},
			{xPos,			yPos,			0.0f, 1.0f},
			{xPos + w,	yPos,			1.0f, 1.0f},

			{xPos,			yPos + h, 0.0f, 0.0f},
			{xPos + w,	yPos,			1.0f, 1.0f},
			{xPos + w,	yPos + h,	1.0f, 0.0f}
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textID);
		// update content of VBO memory
		textVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		textVBO.Unbind();
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// advance cursors for next glyph (advance is number of 1/64 pixels
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}

	textVAO.Unbind();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void MenuSystem::UpdateUIScale()
{
	float screenAspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	float baseAspect = 1280.0f / 720.0f;

	if (screenAspect > baseAspect)
	{
		uniformScale = static_cast<float>(screenHeight) / 720.0f;
		
		offsetX = (screenWidth - (1280.0f * uniformScale)) * 0.5f;
		offsetY = 0.0f;
	}
	else
	{
		uniformScale = static_cast<float>(screenWidth) / 1280.0f;

		offsetX = 0.0f;
		offsetY = (screenHeight - 720.0f * uniformScale) * 0.5f;
	}
	scaleX = uniformScale;
	scaleY = uniformScale;
}

float MenuSystem::ScaledX(float x)
{
	return x * scaleX + offsetX;
}

float MenuSystem::ScaledY(float y)
{
	return y * scaleY + offsetY;
}

float MenuSystem::ScaledWidth(float width)
{
	return width * scaleX;
}

float MenuSystem::ScaledHeight(float height)
{
	return height * scaleY;
}

glm::vec2 MenuSystem::ScaledPosition(float x, float y)
{
	return glm::vec2(ScaledX(x), ScaledY(y));
}

glm::vec2 MenuSystem::ScaledSize(float width, float height)
{
	return glm::vec2(ScaledWidth(width), ScaledHeight(height));
}

void MenuSystem::WindowSizeListener(Event& e)
{
	screenWidth = e.GetParam<unsigned int>(Events::Window::Resized::WIDTH);
	screenHeight = e.GetParam<unsigned int>(Events::Window::Resized::HEIGHT);
	fonts.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));

	textShader->use();
	textShader->setMat4("u_projection", fonts.projMat);

	uiShader->use();
	uiShader->setMat4("u_projection", fonts.projMat);

	UpdateUIScale();
}

void MenuSystem::KeyboardInputListener(Event& e)
{
	int keyRecieve = e.GetParam<int>(Events::Window::Input::KEY);
	int action = e.GetParam<bool>(Events::Window::Input::ACTION);
	char key = static_cast<char>(keyRecieve);

	if (canNavigate && currentStateGlobal == GameState::STARTMENU)
	{
		if (key == Keys::KEY_FORWARD && action == true)
		{
			if (currentHover > 0)
			{
				Event event(Events::Audio::PLAY_SOUND);
				event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
				event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
				event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
				controller.SendEvent(event);

				currentHover--;
			}
			canNavigate = false;
		}
		if (key == Keys::KEY_BACKWARD && action == true)
		{
			if (currentHover < 2)
			{
				Event event(Events::Audio::PLAY_SOUND);
				event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
				event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
				event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
				controller.SendEvent(event);

				currentHover++;
			}
			canNavigate = false;
		}
	}

	if (key == Keys::KEY_JUMP && action == true && currentStateGlobal == GameState::STARTMENU)
	{
		switch (currentHover)
		{
		case Menus::START:
		{
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::CONTROLS);
			controller.SendEvent(event);
			break;
		}
		case Menus::SETTINGS:
		{
			// settings menu
			break;
		}
		case Menus::QUIT:
		{
			// quit game
			controller.SendEvent(Events::Window::CLOSE);
			break;
		}
		}
	}

	else if (key == Keys::KEY_JUMP && action == true && currentStateGlobal == GameState::ENDMENU) {
		playerWon = false;
		Event event(Events::GameState::NEW_STATE);
		event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::STARTMENU);
		controller.SendEvent(event);
	}

	else if (key == Keys::KEY_JUMP && action == true && currentStateGlobal == GameState::CONTROLS) {
		Event event(Events::GameState::NEW_STATE);
		event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::GAME);
		controller.SendEvent(event);
	}
}
