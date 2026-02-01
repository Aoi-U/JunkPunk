#pragma once

#include "System.h"
#include "../Core/Shader.h"
#include "../Core/VAO.h"
#include "../Core/VBO.h"
#include "../Core/Text.h"
#include "../Components/Camera.h"
#include "../../Gamepad.h"

class Event;

class MenuSystem : public System
{
public:
	MenuSystem();

	void Init(std::shared_ptr<Gamepad> gamepad);
	void Update();


private:
	void Clear(float r, float g, float b, float a); // clears screen and buffers
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

	void WindowSizeListener(Event& e);

	unsigned int screenWidth = 1280;
	unsigned int screenHeight = 720;

	glm::mat4 view = glm::mat4(1.0f);

	Text fonts;
	VAO textVAO;
	VBO textVBO;
	std::shared_ptr<Shader> textShader;

	std::shared_ptr<Gamepad> gamepad;
};