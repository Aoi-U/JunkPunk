#include "PauseSystem.h"
#include "../ECSController.h"

extern ECSController controller;

PauseSystem::PauseSystem()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	defaultColor = glm::vec3(1.0f, 1.0f, 1.0f);
	hoverColor = glm::vec3(0.3f, 1.0f, 0.3f);

	textShader = std::make_unique<Shader>("assets/shaders/text.vert", "assets/shaders/text.frag");

	controller.AddEventListener(Events::Window::RESIZED, [this](Event& e) {this->WindowSizeListener(e); });
	controller.AddEventListener(Events::Window::INPUT, [this](Event& e) { this->KeyboardInputListener(e); });
}

void PauseSystem::Init(std::vector<std::shared_ptr<Gamepad>> gamepad)
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

	uiShader = std::make_shared<Shader>("assets/shaders/ui.vert", "assets/shaders/ui.frag");

	uiShader->use();
	uiShader->setMat4("u_projection", fonts.projMat);
	uiShader->setInt("u_texture", 0);

	Texture tex("Powerup.png");
	tex.Load("assets/textures");

	powerupUIElements.emplace_back(
		0.0f,
		0.0f,
		1280.0f,
		720.0f,
		glm::vec4(1.0f),
		ScaleMode::FILL,
		std::make_unique<Texture>(tex)
	);
}

void PauseSystem::Update()
{
	// Helper: check if any gamepad has the button down
	auto AnyButtonDown = [&](int button) -> bool {
		for (auto& gp : gamepad)
		{
			if (gp->Connected() && gp->GetButtonDown(button))
				return true;
		}
		return false;
	};

	// Helper: check if all gamepads are in deadzone
	auto AllSticksInDeadzone = [&]() -> bool {
		for (auto& gp : gamepad)
		{
			if (gp->Connected() && !gp->LStick_InDeadzone())
				return false;
		}
		return true;
	};

	// Helper: get the first non-deadzone left stick X value
	auto AnyLeftStickX = [&]() -> float {
		for (auto& gp : gamepad)
		{
			if (gp->Connected() && !gp->LStick_InDeadzone())
				return gp->LeftStick_X();
		}
		return 0.0f;
	};

	if (showingPowerupScreen)
	{
		glDisable(GL_DEPTH_TEST);

		RenderElements(powerupUIElements);

		if (AnyButtonDown(Buttons::JUMP))
		{
			showingPowerupScreen = false;
		}

		return;
	}

	if (AllSticksInDeadzone())
	{
		canNavigate = true;
	}

	if (canNavigate)
	{
		float stickX = AnyLeftStickX();

		if (stickX < -0.5f) // navigate left
		{
			// navigate left
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
		else if (stickX > 0.5f) // navigate right
		{
			if (currentHover < 3)
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
	}

	if (AnyButtonDown(Buttons::JUMP))
	{
		switch (currentHover)
		{
		case Menus::RESUME:
		{

			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::GAME);
			controller.SendEvent(event);
			break;
		}
		case Menus::RESTART:
		{
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::RESTART);
			controller.SendEvent(event);
			break;
		}
		case Menus::MENU:
		{
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::STARTMENU);
			controller.SendEvent(event);
			break;
		}
		case Menus::POWERUPS:
		{
			showingPowerupScreen = true;
			break;
		}
		}
	}

	float scale = 1.0f;
	float spacing = 100.0f;

	std::string title = "PAUSED";
	RenderText(
		title,
		GetCenteredX(title, scale * 1.5f),
		0.65f * screenHeight,
		scale * 1.5f,
		glm::vec3(1.0f, 1.0f, 1.0f)
	);

	float y = 0.45f * screenHeight;

	auto getWidth = [&](const std::string& text)
		{
			float w = 0.0f;
			for (char c : text)
			{
				Character ch = fonts.charArial[c];
				w += (ch.Advance >> 6) * scale;
			}
			return w;
		};

	std::string resumeText = "RESUME";
	std::string restartText = "RESTART";
	std::string powerText = "CONTROLS";
	std::string menuText = "MENU";

	float wResume = getWidth(resumeText);
	float wRestart = getWidth(restartText);
	float wPower = getWidth(powerText);
	float wMenu = getWidth(menuText);

	float totalWidth =
		wResume + wRestart + wPower + wMenu +
		spacing * 3;

	float startX = (screenWidth * 0.5f) - (totalWidth * 0.5f);

	float x = startX;

	RenderText(
		resumeText,
		x,
		y,
		scale,
		currentHover == Menus::RESUME ? hoverColor : defaultColor
	);
	x += wResume + spacing;

	RenderText(
		restartText,
		x,
		y,
		scale,
		currentHover == Menus::RESTART ? hoverColor : defaultColor
	);
	x += wRestart + spacing;

	RenderText(
		powerText,
		x,
		y,
		scale,
		currentHover == Menus::POWERUPS ? hoverColor : defaultColor
	);
	x += wPower + spacing;

	RenderText(
		menuText,
		x,
		y,
		scale,
		currentHover == Menus::MENU ? hoverColor : defaultColor
	);
}

void PauseSystem::Reset()
{
	canNavigate = true;
	currentHover = 0;
}

void PauseSystem::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
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

void PauseSystem::WindowSizeListener(Event& e)
{
	screenWidth = e.GetParam<unsigned int>(Events::Window::Resized::WIDTH);
	screenHeight = e.GetParam<unsigned int>(Events::Window::Resized::HEIGHT);
	fonts.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
	textShader->use();
	textShader->setMat4("u_projection", fonts.projMat);
}

void PauseSystem::KeyboardInputListener(Event& e)
{
	int keyRecieve = e.GetParam<int>(Events::Window::Input::KEY);
	int action = e.GetParam<bool>(Events::Window::Input::ACTION);
	char key = static_cast<char>(keyRecieve);

	if (showingPowerupScreen) {
		if (key == Keys::KEY_JUMP && action == true)
		{
			showingPowerupScreen = false;
			return;
		}
	}

	if (canNavigate && currentStateGlobal == GameState::PAUSED)
	{
		if (key == Keys::KEY_LEFT && action == true)
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
		else if (key == Keys::KEY_RIGHT && action == true)
		{
			if (currentHover < 3)
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

	if (key == Keys::KEY_JUMP && action == true && currentStateGlobal == GameState::PAUSED)
	{
		switch (currentHover)
		{
		case Menus::RESUME:
		{

			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::GAME);
			controller.SendEvent(event);
			break;
		}
		case Menus::RESTART:
		{
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::RESTART);
			controller.SendEvent(event);
			break;
		}
		case Menus::MENU:
		{
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::STARTMENU);
			controller.SendEvent(event);
			break;
		}
		case Menus::POWERUPS:
		{
			showingPowerupScreen = true;
			break;
		}
		}
	}
}

float PauseSystem::GetCenteredX(std::string text, float scale)
{
	float width = 0.0f;

	for (char c : text)
	{
		Character ch = fonts.charArial[c];
		width += (ch.Advance >> 6) * scale;
	}

	return (screenWidth * 0.5f) - (width * 0.5f);
}

void PauseSystem::RenderElements(std::vector<UIElement>& elements)
{
	for (auto& elem : elements)
	{
		if (elem.tex)
		{
			uiShader->use();
			uiShader->setVec4("u_color", &elem.color.x);
			uiShader->setInt("u_useTexture", 1);
			uiShader->setMat4("u_projection", fonts.projMat);

			float vertices[6][4] = {
				{ 0.0f, screenHeight, 0.0f, 0.0f },
				{ 0.0f, 0.0f,         0.0f, 1.0f },
				{ screenWidth, 0.0f,  1.0f, 1.0f },

				{ 0.0f, screenHeight, 0.0f, 0.0f },
				{ screenWidth, 0.0f,  1.0f, 1.0f },
				{ screenWidth, screenHeight, 1.0f, 0.0f }
			};

			elem.tex->Bind(GL_TEXTURE0);

			uiVAO.Bind();
			uiVBO.Bind();
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			uiVAO.Unbind();
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}