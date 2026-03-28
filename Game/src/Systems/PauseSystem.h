#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "System.h"
#include "../Core/Shader.h"
#include "../Core/Types.h"
#include "../Core/Text.h"
#include "../../Gamepad.h"
#include "../Core/VAO.h"
#include "../Core/VBO.h"
#include "../Core/Texture.h"
#include "MenuSystem.h"


class Event;

class PauseSystem : public System
{
public:
	PauseSystem();
	void Init(std::vector<std::shared_ptr<Gamepad>> gamepad);
	void Update();
	void Reset();

private:
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

	void WindowSizeListener(Event& e);
	void KeyboardInputListener(Event& e);

	void RenderElements(std::vector<UIElement>& elements);

	float GetCenteredX(std::string text, float scale);

	enum Menus
	{
		RESUME,
		RESTART,
		POWERUPS,
		MENU
	};

	unsigned int screenWidth = 1280;
	unsigned int screenHeight = 720;

	Text fonts;
	VAO textVAO;
	VBO textVBO;
	std::shared_ptr<Shader> textShader;

	std::vector<std::shared_ptr<Gamepad>> gamepad;
	bool canNavigate = true;
	int currentHover = 0;
	glm::vec3 defaultColor = glm::vec3(0.5f, 0.8f, 0.2f);
	glm::vec3 hoverColor = glm::vec3(0.7f, 1.0f, 0.2f);

	std::vector<UIElement> powerupUIElements;
	bool showingPowerupScreen = false;

	VAO uiVAO;
	VBO uiVBO;
	std::shared_ptr<Shader> uiShader;
};
