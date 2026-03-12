#pragma once

#include "System.h"
#include "../Core/Shader.h"
#include "../Core/VAO.h"
#include "../Core/VBO.h"
#include "../Core/Text.h"
#include "../Core/Texture.h"
#include "../../Gamepad.h"

class Event;

enum ScaleMode
{
	FIT,
	FILL,
	STRETCH
};

struct UIElement
{
	float x, y; // position
	float width, height; // size
	glm::vec4 color; // color
	ScaleMode scaleMode = FIT;
	std::unique_ptr<Texture> tex; // texture (optional)

	UIElement(float x, float y, float width, float height, glm::vec4 color, ScaleMode scaleMode = ScaleMode::FIT, std::unique_ptr<Texture> tex = nullptr)
		: x(x), y(y), width(width), height(height), color(color), scaleMode(scaleMode), tex(std::move(tex))
	{}
};

class MenuSystem : public System
{
public:
	MenuSystem();

	void Init(std::shared_ptr<Gamepad> gamepad);
	void Update();
	void Reset();
	void RenderWinText();
	void RenderFadeOverlay(float alpha);

private:
	void Clear(float r, float g, float b, float a); // clears screen and buffers

	void RenderElements(std::vector<UIElement>& elements);
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
	void RenderUIRect(float x, float y, float width, float height, glm::vec4 color);
	void RenderUITexture(float x, float y, float width, float height, Texture* tex, glm::vec4 tint);
	void RenderEndScreen();
	void RenderControlsScreen();
	void RenderSettingsScreen();

	void InitUI();
	void InitEndUI();
	void InitControlsUI();
	void InitSettingsUI();

	void UpdateUIScale();
	float ScaledX(float x);
	float ScaledY(float y);
	float ScaledWidth(float width);
	float ScaledHeight(float height);
	glm::vec2 ScaledPosition(float x, float y);
	glm::vec2 ScaledSize(float width, float height);

	void WindowSizeListener(Event& e);
	void KeyboardInputListener(Event& e);

	enum Menus
	{
		START,
		SETTINGS,
		QUIT,

		PLAYER_COUNT
	};

	unsigned int screenWidth = 1280;
	unsigned int screenHeight = 720;

	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float uniformScale = 1.0f;
	float offsetX = 0.0f;
	float offsetY = 0.0f;

	glm::mat4 view = glm::mat4(1.0f);

	Text fonts;
	VAO textVAO;
	VBO textVBO;
	std::shared_ptr<Shader> textShader;

	VAO uiVAO;
	VBO uiVBO;
	std::shared_ptr<Shader> uiShader;

	std::shared_ptr<Gamepad> gamepad;
	bool canNavigate = falses;
	int currentHover = 0;
	int maxVerticalHover = 2;


	glm::vec3 defaultColor = glm::vec3(0.5f, 0.8f, 0.2f);
	glm::vec3 hoverColor = glm::vec3(0.7f, 1.0f, 0.2f);

	std::vector<UIElement> uiElements;
	std::vector<UIElement> endUIElements;
	std::vector<UIElement> controlsUIElements;
	std::vector<UIElement> settingsUIElements;

};