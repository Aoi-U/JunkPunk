#pragma once
#include <memory>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unordered_map>

#include "Core/Types.h"

struct WindowDeleter {
	void operator() (GLFWwindow* window) const {
		glfwDestroyWindow(window);
	}
};

class Window
{
public:
	Window(int width, int height, const char* title);

	int shouldClose() { return glfwWindowShouldClose(window.get()); }
	void makeContextCurrent() { glfwMakeContextCurrent(window.get()); }
	void swapBuffers() { glfwSwapBuffers(window.get()); }

	void setCallbacks();


	GLFWwindow* getGLFWwindow() const { return window.get(); }

	std::pair<int, int> getFrameBufferSize() const {
		return { fbWidth, fbHeight };
	}

private:
	std::unique_ptr<GLFWwindow, WindowDeleter> window;

	static int fbWidth;
	static int fbHeight;
	static std::unordered_map<int, bool> mKeyStatusMap;
	static std::unordered_map<int, bool> mMouseStatusMap;

	static void defaultWindowSizeCallback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	static void keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosMetaCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset);
};
