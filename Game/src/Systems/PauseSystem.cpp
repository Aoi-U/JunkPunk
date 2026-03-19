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

void PauseSystem::Init(std::shared_ptr<Gamepad> gamepad)
{
	this->gamepad = gamepad;

	fonts = Text();
	textVAO = VAO();
	textVBO = VBO();
	fonts.initVAO(&textVAO, &textVBO);
	fonts.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));

	textShader->use();
	textShader->setMat4("u_projection", fonts.projMat);
}

void PauseSystem::Update()
{
	if (gamepad->LStick_InDeadzone())
	{
		canNavigate = true;
	}

	if (canNavigate)
	{
		if (gamepad->LeftStick_X() < -0.5f) // navigate left
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
		else if (gamepad->LeftStick_X() > 0.5f) // navigate right
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
		}
	}

	float scale = 1.0f;
	float spacing = 200.0f;

	std::string title = "PAUSED";
	RenderText(
		title,
		GetCenteredX(title, scale * 1.5f),
		0.65f * screenHeight,
		scale * 1.5f,
		glm::vec3(1.0f, 1.0f, 1.0f)
	);

	float y = 0.45f * screenHeight;

	std::string resumeText = "RESUME";
	std::string restartText = "RESTART";
	std::string menuText = "MENU";

	RenderText(
		restartText,
		GetCenteredX(restartText, scale),
		y,
		scale,
		currentHover == Menus::RESTART ? hoverColor : defaultColor
	);

	RenderText(
		resumeText,
		GetCenteredX(resumeText, scale) - spacing,
		y,
		scale,
		currentHover == Menus::RESUME ? hoverColor : defaultColor
	);

	RenderText(
		menuText,
		GetCenteredX(menuText, scale) + spacing,
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