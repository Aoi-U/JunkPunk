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
	InitSettingsUI();
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
		if (gamepad->LeftStick_Y() > 0.3f) // navigate up
		{
			// navigate up
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
		else if (gamepad->LeftStick_Y() < -0.3f) // navigate down
		{
			if (currentHover < maxVerticalHover)
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

		// add horizontal navigation for settings
		if (currentStateGlobal == GameState::SETTINGS)
		{
			switch (currentHover)
			{
			case SettingsMenu::PLAYER_COUNT:
				// horizontal navigation for player count
				if (gamepad->LeftStick_X() > 0.3f) // increase player count
				{
					if (numPlayers < maxPlayerCount) // assuming max 4 players
					{
						Event event(Events::Audio::PLAY_SOUND);
						event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
						event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
						event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
						controller.SendEvent(event);
						numPlayers++;
					}
					canNavigate = false;
				}
				else if (gamepad->LeftStick_X() < -0.3f) // decrease player count
				{
					if (numPlayers > 1) // minimum 1 player
					{
						Event event(Events::Audio::PLAY_SOUND);
						event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
						event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
						event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
						controller.SendEvent(event);
						numPlayers--;
					}
					canNavigate = false;
				}

				break;

			case SettingsMenu::AI_COUNT:
				// horizontal navigation for AI count
				if (gamepad->LeftStick_X() > 0.3f) // increase AI count
				{
					if (numAi < maxAICount) 
					{
						Event event(Events::Audio::PLAY_SOUND);
						event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
						event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
						event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
						controller.SendEvent(event);
						numAi++;
					}
					canNavigate = false;
				}
				else if (gamepad->LeftStick_X() < -0.3f) // decrease AI count
				{
					if (numAi > 0) 
					{
						Event event(Events::Audio::PLAY_SOUND);
						event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
						event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
						event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
						controller.SendEvent(event);
						numAi--;
					}
					canNavigate = false;
				}
			}
		}
	}

	glDisable(GL_DEPTH_TEST);
	Clear(0.1f, 0.1f, 0.1f, 1.0f);

	// render different menu screens based on current game state
	switch (currentStateGlobal)
	{
	case GameState::STARTMENU:
	{
		RenderElements(uiElements);
		float buttonSpacing = 120.0f;
		float textScale = uniformScale;

		float startY = ScaledY(420.0f);
		std::string startText = "START";
		std::string settingsText = "SETTINGS";
		std::string quitText = "QUIT";

		RenderText(startText,
			GetCenteredX(startText, textScale),
			startY,
			textScale,
			currentHover == Menus::START ? hoverColor : defaultColor);

		RenderText(settingsText,
			GetCenteredX(settingsText, textScale),
			startY - buttonSpacing,
			textScale,
			currentHover == Menus::SETTINGS ? hoverColor : defaultColor);

		RenderText(quitText,
			GetCenteredX(quitText, textScale),
			startY - buttonSpacing * 2,
			textScale,
			currentHover == Menus::QUIT ? hoverColor : defaultColor);
		
		// handle button press for current selection in main menu
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
				currentHover = 0;
				maxVerticalHover = 1;

				Event event(Events::GameState::NEW_STATE);
				event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::SETTINGS);
				controller.SendEvent(event);
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
		break;
	}
	case GameState::ENDMENU:
	{
		RenderEndScreen();
		break;
	}
	case GameState::CONTROLS:
	{
		RenderControlsScreen();

		// start game when A is pressed on controls screen
		if (gamepad->GetButtonDown(Buttons::JUMP)) {
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::GAME);
			controller.SendEvent(event);
		}
		break;
	}
	case GameState::SETTINGS:
	{
		RenderSettingsScreen();

		// return to main menu when B is pressed on settings screen
		if (gamepad->GetButtonDown(Buttons::POWERUP))
		{
			currentHover = 0;
			maxVerticalHover = 2;

			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::STARTMENU);
			controller.SendEvent(event);
		}

		break;
	}
	}
}

void MenuSystem::Reset()
{
	canNavigate = true;
	currentHover = 0;
}

void MenuSystem::RenderWinText() {
	glDisable(GL_DEPTH_TEST);
	float scale = uniformScale * 2.0f;
	if (!playerWon && !aiWon)
		return;
	if (playerWon) {
		std::string text = numPlayers > 1
			? "PLAYER " + std::to_string(winningPlayerNum) + " WINS!"
			: "YOU WIN!";
		RenderText(text, GetCenteredX(text, scale), ScaledY(600.0f), scale, glm::vec3(0.2f, 1.0f, 0.2f));
	}
	else {
		std::string text = "OPPONENT WINS!";
		RenderText(text, GetCenteredX(text, scale), ScaledY(600.0f), scale, glm::vec3(1.0f, 0.2f, 0.2f));
	}
}

void MenuSystem::RenderEndScreen() {
	glDisable(GL_DEPTH_TEST);
	Clear(0.05f, 0.05f, 0.05f, 1.0f);
	float titleScale = uniformScale * 2.0f;
	
	Texture* bg = nullptr;

	if (playerWon)
		bg = winBackground.get();
	else if (aiWon)
		bg = loseBackground.get();
	if (bg) {
		RenderUITexture(
			0.0f,
			0.0f,
			(float)screenWidth,
			(float)screenHeight,
			bg,
			glm::vec4(1.0f)
			);
	}
	else
		RenderElements(endUIElements);

	if (aiWon) 	{
		 //titleScale = uniformScale * 1.5f;
		std::string text = "OPPONENT WINS!";
		RenderText(
			text,
			GetCenteredX(text, titleScale),
			ScaledY(600.0f),
			titleScale,
			glm::vec3(1.0f, 0.2f, 0.2f)
		);
	}
	else if (playerWon){
		std::string text = numPlayers > 1
			? "PLAYER " + std::to_string(winningPlayerNum) + " WINS!"
			: "YOU WIN!";
		RenderText(
			text,
			GetCenteredX(text, titleScale),
			ScaledY(600.0f),
			titleScale,
			glm::vec3(0.2f, 1.0f, 0.2f)
		);
	}

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
	RenderElements(controlsUIElements);
}

void MenuSystem::RenderSettingsScreen()
{
	RenderElements(settingsUIElements);

	float buttonSpacing = 120.0f;
	float textScale = uniformScale;
	RenderText("Player Count: " + std::to_string(numPlayers), ScaledX(500.0f), ScaledY(500.0f), textScale, currentHover == SettingsMenu::PLAYER_COUNT ? hoverColor : defaultColor);
	RenderText("AI Count: " + std::to_string(numAi), ScaledX(500.0f), ScaledY(370.0f), textScale, currentHover == SettingsMenu::AI_COUNT ? hoverColor : defaultColor);
	

	// back instruction
	RenderText("Press B to return to Main Menu", ScaledX(0.0f), ScaledY(700.0f), textScale * 0.5, glm::vec3(1.0f));
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
	Texture tex("JunkPunk intro.png");	
	tex.Load("assets/textures");
	uiElements.emplace_back(0.0f, 0.0f, 1280.0f, 720.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), ScaleMode::FILL, std::make_unique<Texture>(tex));
}

void MenuSystem::InitEndUI() {
	endUIElements.clear();
	Texture tex("dumpster sunset.jpg");
	tex.Load("assets/textures");

	winBackground = std::make_unique<Texture>("JunkPunk win.png");
	winBackground->Load("assets/textures");

	loseBackground = std::make_unique<Texture>("JunkPunk lose.png");
	loseBackground->Load("assets/textures");

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

void MenuSystem::InitSettingsUI()
{
	float buttonWidth = 300.0f;
	float buttonHeight = 80.0f;
	float buttonX = 640.0f - buttonWidth * 0.5f;
	float buttonSpacing = 120.0f;

	float playerCountY = 500.0f - buttonHeight * 0.5f;
	float aiCountY = playerCountY - buttonSpacing;

	settingsUIElements.clear();
	settingsUIElements.emplace_back(buttonX, playerCountY, buttonWidth, buttonHeight, glm::vec4(0.2f, 0.2f, 0.2f, 0.9f));
	settingsUIElements.emplace_back(buttonX, aiCountY, buttonWidth, buttonHeight, glm::vec4(0.2f, 0.2f, 0.2f, 0.9f));

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
	else if (canNavigate && currentStateGlobal == GameState::SETTINGS)
	{
		if (key == Keys::KEY_FORWARD && action == true)
		{
			if (currentHover > 0)
			{
				Event event(Events::Audio::PLAY_SOUND);
				event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
				event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3 { 0.0f, 0.0f, 0.0f });
				event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
				controller.SendEvent(event);

				currentHover--;
			}
			canNavigate = false;
		}
		if (key == Keys::KEY_BACKWARD && action == true)
		{
			if (currentHover < 1)
			{
				Event event(Events::Audio::PLAY_SOUND);
				event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
				event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3 { 0.0f, 0.0f, 0.0f });
				event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
				controller.SendEvent(event);

				currentHover++;
			}
			canNavigate = false;
		}
		// return to main menu when B is pressed on settings screen
		if (key == Keys::KEY_USE && action == true)
		{
			currentHover = 0;
			maxVerticalHover = 2;

			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::STARTMENU);
			controller.SendEvent(event);
		}
	}


	if (canNavigate && currentStateGlobal == GameState::SETTINGS) {
		switch (currentHover)
		{
		case SettingsMenu::PLAYER_COUNT:
			// horizontal navigation for player count
			if (key == Keys::KEY_RIGHT && action == true) // increase player count
			{
				if (numPlayers < maxPlayerCount) // assuming max 4 players
				{
					Event event(Events::Audio::PLAY_SOUND);
					event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
					event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3 { 0.0f, 0.0f, 0.0f });
					event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
					controller.SendEvent(event);
					numPlayers++;
				}
				canNavigate = false;
			}
			else if (key == Keys::KEY_LEFT && action == true) // decrease player count
			{
				if (numPlayers > 1) // minimum 1 player
				{
					Event event(Events::Audio::PLAY_SOUND);
					event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
					event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3 { 0.0f, 0.0f, 0.0f });
					event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
					controller.SendEvent(event);
					numPlayers--;
				}
				canNavigate = false;
			}

			break;

		case SettingsMenu::AI_COUNT:
			// horizontal navigation for AI count
			if (key == Keys::KEY_RIGHT && action == true) // increase AI count
			{
				if (numAi < maxAICount)
				{
					Event event(Events::Audio::PLAY_SOUND);
					event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
					event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3 { 0.0f, 0.0f, 0.0f });
					event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
					controller.SendEvent(event);
					numAi++;
				}
				canNavigate = false;
			}
			else if (key == Keys::KEY_LEFT && action == true) // decrease AI count
			{
				if (numAi > 0)
				{
					Event event(Events::Audio::PLAY_SOUND);
					event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/MenuNavigation.wav");
					event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3 { 0.0f, 0.0f, 0.0f });
					event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, 0.0f);
					controller.SendEvent(event);
					numAi--;
				}
				canNavigate = false;
			}
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
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::SETTINGS);
			controller.SendEvent(event);
			currentHover = 0;
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

float MenuSystem::GetCenteredX(std::string text, float scale)
{
	float width = 0.0f;

	for (char c : text)
	{
		Character ch = fonts.charArial[c];
		width += (ch.Advance >> 6) * scale;
	}

	return (screenWidth * 0.5f) - (width * 0.5f);
}