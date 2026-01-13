#pragma once
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "InputManager.h"

struct WindowDeleter {
	void operator() (GLFWwindow* window) const {
		glfwDestroyWindow(window);
	}
};

class Window
{
public:
	Window(int width, int height, const char* title, std::shared_ptr<InputManager> callbacks);


	glm::ivec2 getWindowPos() const;
	glm::ivec2 getWindowSize() const;

	int shouldClose() { return glfwWindowShouldClose(window.get()); }
	void makeContextCurrent() { glfwMakeContextCurrent(window.get()); }
	void swapBuffers() { glfwSwapBuffers(window.get()); }

	void setCallbacks();


	GLFWwindow* getGLFWwindow() const { return window.get(); }

private:
	std::unique_ptr<GLFWwindow, WindowDeleter> window;
	std::shared_ptr<InputManager> inputManager; 

	static void defaultWindowSizeCallback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

	static void keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosMetaCallback(GLFWwindow* window, double xpos, double ypos);
	static void windowSizeMetaCallback(GLFWwindow* window, int width, int height);
	static void scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset);
};
