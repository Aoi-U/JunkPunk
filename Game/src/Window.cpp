#include "Window.h"

int Window::fbWidth = 0;
int Window::fbHeight = 0;

void Window::keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	InputManager* callbacks = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

	bool forward_to_user_callback = true;
	// Forward the key event to ImGui
	/*if (ImGui::GetCurrentContext()) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		forward_to_user_callback = !ImGui::GetIO().WantCaptureKeyboard;
	}*/

	// If ImGui doesn't want to capture the keyboard, call the user-defined callback
	if (forward_to_user_callback && callbacks) {
		callbacks->keyCallback(key, scancode, action, mods);
	}
}


void Window::mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods) {
	InputManager* callbacks = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

	bool forward_to_user_callback = true;
	// Forward the event to ImGui
	/*if (ImGui::GetCurrentContext()) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		forward_to_user_callback = !ImGui::GetIO().WantCaptureMouse;
	}*/

	// If ImGui doesn't want to capture the mouse, call the user-defined callback
	if (forward_to_user_callback && callbacks) {
		callbacks->mouseButtonCallback(button, action, mods);
	}
}


void Window::cursorPosMetaCallback(GLFWwindow* window, double xpos, double ypos) {
	InputManager* callbacks = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

	bool forward_to_user_callback = true;
	// Forward the event to ImGui
	/*if (ImGui::GetCurrentContext()) {
		ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
		forward_to_user_callback = !ImGui::GetIO().WantCaptureMouse;
	}*/

	// Call user-defined cursor position callback only if ImGui isn't capturing the mouse
	if (forward_to_user_callback && callbacks) {
		callbacks->cursorPosCallback(xpos, ypos);
	}
}

void Window::windowSizeMetaCallback(GLFWwindow* window, int width, int height) {
	InputManager* callbacks = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

	// Call the user-defined callback for window resizing
	if (callbacks) {
		callbacks->windowSizeCallback(width, height);
	}
}

void Window::scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset) {
	InputManager* callbacks = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

	bool forward_to_user_callback = true;
	// Forward the scroll event to ImGui
	//if (ImGui::GetCurrentContext()) {
	//	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	//	forward_to_user_callback = !ImGui::GetIO().WantCaptureMouse;
	//}

	// If ImGui isn't capturing scroll input, call the user-defined scroll callback
	if (forward_to_user_callback && callbacks) {
		callbacks->scrollCallback(xoffset, yoffset);
	}
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	fbWidth = width;
	fbHeight = height;
	glViewport(0, 0, width, height);
}

Window::Window(int width, int height, const char* title, std::shared_ptr<InputManager> im)
	: window(nullptr), inputManager(im)
{
	// specify OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

	// create window
	window = std::unique_ptr<GLFWwindow, WindowDeleter>(glfwCreateWindow(width, height, title, NULL, NULL));
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window." << std::endl;
		return;
	}
	makeContextCurrent();
	glfwSetFramebufferSizeCallback(window.get(), framebuffer_size_callback);
	glfwGetFramebufferSize(window.get(), &fbWidth, &fbHeight);

	// initialize OpenGL extensions for the current context (this window)
	if (!gladLoadGL()) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}	

	glfwSetWindowSizeCallback(window.get(), defaultWindowSizeCallback);

	setCallbacks();
}

glm::ivec2 Window::getWindowPos() const {
	int x, y;
	glfwGetWindowPos(window.get(), &x, &y);
	return glm::ivec2(x, y);
}


glm::ivec2 Window::getWindowSize() const {
	int w, h;
	glfwGetWindowSize(window.get(), &w, &h);
	return glm::ivec2(w, h);
}

void Window::setCallbacks()
{
	glfwSetWindowUserPointer(window.get(), inputManager.get());

	glfwSetKeyCallback(window.get(), keyMetaCallback);
	glfwSetWindowSizeCallback(window.get(), windowSizeMetaCallback);
	glfwSetMouseButtonCallback(window.get(), mouseButtonMetaCallback);
	glfwSetCursorPosCallback(window.get(), cursorPosMetaCallback);
	glfwSetScrollCallback(window.get(), scrollMetaCallback);
}

