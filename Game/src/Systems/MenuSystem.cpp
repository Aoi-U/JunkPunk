#include "MenuSystem.h"

#include "../ECSController.h"

extern ECSController controller;

MenuSystem::MenuSystem()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	textShader = std::make_unique<Shader>("assets/shaders/text.vert", "assets/shaders/text.frag");

	controller.AddEventListener(Events::Window::RESIZED, [this](Event& e) {this->WindowSizeListener(e); });
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
		case Menus::START:
		{
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::GAME);
			controller.SendEvent(event);
		}
		break;
		case Menus::SETTINGS:
		{
			// settings menu
		}
		break;
		case Menus::QUIT:
		{
			// quit game
			controller.SendEvent(Events::Window::CLOSE);
		}
		break;
		}
	}



	Clear(0.1f, 0.1f, 0.1f, 1.0f);
	RenderText("START", 0.2 * screenWidth, 0.5 * screenHeight, 1.0f, currentHover == Menus::START ? hoverColor : defaultColor);
	RenderText("SETTINGS", 0.4 * screenWidth, 0.5 * screenHeight, 1.0f, currentHover == Menus::SETTINGS ? hoverColor : defaultColor);
	RenderText("QUIT", 0.7 * screenWidth, 0.5 * screenHeight, 1.0f, currentHover == Menus::QUIT ? hoverColor : defaultColor);
}

void MenuSystem::Reset()
{
	canNavigate = true;
	currentHover = 0;
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

void MenuSystem::WindowSizeListener(Event& e)
{
	screenWidth = e.GetParam<unsigned int>(Events::Window::Resized::WIDTH);
	screenHeight = e.GetParam<unsigned int>(Events::Window::Resized::HEIGHT);
	fonts.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
	textShader->use();
	textShader->setMat4("u_projection", fonts.projMat);
}
