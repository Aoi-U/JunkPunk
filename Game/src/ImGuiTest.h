#pragma once

#include <memory>

#include "Window.h"

// ImGui for testing purposes

class ImGuiTest
{
public:
	ImGuiTest(std::shared_ptr<Window> window);

	void Render();
	void Shutdown();

private:
	// window
	std::shared_ptr<Window> window;
};