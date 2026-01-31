#pragma once

#include "System.h"
#include "../Core/Shader.h"
#include "../Core/VAO.h"
#include "../Core/VBO.h"
#include "../Core/Text.h"
#include "../Components/Camera.h"

class Event;

class MenuRenderSystem : public System
{
public:
	MenuRenderSystem();

	void Clear(float r, float g, float b, float a); // clears screen and buffers
	void Init();
	void Update(float fps);


private:
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
	unsigned int screenWidth = 1280;
	unsigned int screenHeight = 720;
	Text fonts;
	VAO textVAO;
	VBO textVBO;
	std::shared_ptr<Shader> textShader;
};