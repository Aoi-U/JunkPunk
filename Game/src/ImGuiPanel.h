#pragma once
#include <glad/glad.h>

#include <memory>
#include <imgui/imgui.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "Window.h"

// Class that specifies render call for Panel
class ImGuiPanelRendererInterface {
public:
	virtual void render() {}
};

// Functor to safely destroy ImGui contexts
struct ImGuiPanelDeleter {
	void operator()(ImGuiContext* context) const {
		ImGui::DestroyContext(context);
	}
};

// The Panel class manages the lifecycle of ImGui with GLFW integration
class ImGuiPanel {
public:
	// multi_view_port_enabled allows the panel to be moved out of the window, though it seems to be a bit buggy on some machines
	ImGuiPanel(std::shared_ptr<Window> share);
	ImGuiPanel(std::shared_ptr<ImGuiPanelRendererInterface> panelRender, std::shared_ptr<Window> share);
	~ImGuiPanel();

	void setPanelRenderer(std::shared_ptr<ImGuiPanelRendererInterface> panelRenderer);

	void render();        // Render content here

private:
	std::unique_ptr<ImGuiContext, ImGuiPanelDeleter> imguiContext;  // Manage ImGui context
	std::shared_ptr<ImGuiPanelRendererInterface> panelRenderer;      // optional shared owning ptr (user provided)

};
