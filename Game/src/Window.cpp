#include "Window.h"
#include "ECSController.h"

int Window::fbWidth = 0;
int Window::fbHeight = 0;

extern ECSController controller;

void Window::keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	bool forward_to_user_callback = true;
	// Forward the key event to ImGui
	/*if (ImGui::GetCurrentContext()) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		forward_to_user_callback = !ImGui::GetIO().WantCaptureKeyboard;
	}*/

	// If ImGui doesn't want to capture the keyboard, call the user-defined callback
	
}


void Window::mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods) {

	bool forward_to_user_callback = true;
	// Forward the event to ImGui
	/*if (ImGui::GetCurrentContext()) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		forward_to_user_callback = !ImGui::GetIO().WantCaptureMouse;
	}*/

	// If ImGui doesn't want to capture the mouse, call the user-defined callback
	
}


void Window::cursorPosMetaCallback(GLFWwindow* window, double xpos, double ypos) {

	bool forward_to_user_callback = true;
	// Forward the event to ImGui
	/*if (ImGui::GetCurrentContext()) {
		ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
		forward_to_user_callback = !ImGui::GetIO().WantCaptureMouse;
	}*/

	// Call user-defined cursor position callback only if ImGui isn't capturing the mouse

}

void Window::scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset) {

	bool forward_to_user_callback = true;
	// Forward the scroll event to ImGui
	//if (ImGui::GetCurrentContext()) {
	//	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	//	forward_to_user_callback = !ImGui::GetIO().WantCaptureMouse;
	//}

	// If ImGui isn't capturing scroll input, call the user-defined scroll callback

}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	fbWidth = width;
	fbHeight = height;
	glViewport(0, 0, width, height);

	Event event(Events::Window::RESIZED);
	event.SetParam<unsigned int>(Events::Window::Resized::WIDTH, static_cast<unsigned int>(width));
	event.SetParam<unsigned int>(Events::Window::Resized::HEIGHT, static_cast<unsigned int>(height));
	controller.SendEvent(event);
}

Window::Window(int width, int height, const char* title)
	: window(nullptr)
{
	// specify OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA


	// create window
	window = std::unique_ptr<GLFWwindow, WindowDeleter>(glfwCreateWindow(width, height, title, NULL, NULL));
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window." << std::endl;
		return;
	}
	makeContextCurrent();
	

	// initialize OpenGL extensions for the current context (this window)
	if (!gladLoadGL()) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}	


	setCallbacks();
}


void Window::setCallbacks()
{
	glfwSetFramebufferSizeCallback(window.get(), framebuffer_size_callback);
	glfwSetWindowSizeCallback(window.get(), defaultWindowSizeCallback);

	glfwGetFramebufferSize(window.get(), &fbWidth, &fbHeight);
	glfwSetKeyCallback(window.get(), keyMetaCallback);
	glfwSetMouseButtonCallback(window.get(), mouseButtonMetaCallback);
	glfwSetCursorPosCallback(window.get(), cursorPosMetaCallback);
	glfwSetScrollCallback(window.get(), scrollMetaCallback);
}

