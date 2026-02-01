#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "System.h"
#include "../Core/Shader.h"
#include "../Core/Types.h"
#include "../Core/Text.h"
#include "../../Gamepad.h"


class Event;

class PauseSystem : public System
{
public:
	PauseSystem();
	void Init(std::shared_ptr<Gamepad> gamepad);
	void Update();

private:
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

	void WindowSizeListener(Event& e);

	unsigned int screenWidth = 1280;
	unsigned int screenHeight = 720;

	Text fonts;
	VAO textVAO;
	VBO textVBO;
	std::shared_ptr<Shader> textShader;

	std::shared_ptr<Gamepad> gamepad;
};
