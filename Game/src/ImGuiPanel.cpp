#include "ImGuiPanel.h"
#include <imgui/imgui_internal.h>



ImGuiPanel::ImGuiPanel(std::shared_ptr<ImGuiPanelRendererInterface> panelRender, std::shared_ptr<Window> share)
	: imguiContext(ImGui::CreateContext()), panelRenderer(panelRender)  // Create ImGui context
{

	ImGui::SetCurrentContext(imguiContext.get());
	ImGui_ImplGlfw_InitForOpenGL(share->getGLFWwindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330");
	// Set ImGui style
	ImGui::StyleColorsDark();

}

ImGuiPanel::ImGuiPanel(std::shared_ptr<Window> share) : ImGuiPanel(nullptr, share) {}

ImGuiPanel::~ImGuiPanel() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
}

void ImGuiPanel::setPanelRenderer(std::shared_ptr<ImGuiPanelRendererInterface> panelRenderer_) {
	panelRenderer = panelRenderer_;
}

void ImGuiPanel::render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (panelRenderer) {  // Check if the panel renderer is set
		panelRenderer->render();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
