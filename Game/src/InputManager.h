#pragma once

#include <functional>
#include<memory>

#include <iostream>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include "glm/glm.hpp"


class InputManager
{
public:

	InputManager();

  ~InputManager();

	bool IsKeyboardButtonDown(int keyboardButton) const;

	bool IsMouseButtonDown(int mouseButton) const;

	glm::dvec2 const& CursorPosition() const;

	int ScrollValueChanged();

  void keyCallback(int key, int scancode, int action, int mods);

  void mouseButtonCallback(int button, int action, int mods);

  void cursorPosCallback(double xpos, double ypos);

  void scrollCallback(double xoffset, double yoffset);

  bool IsWindowFocused() const { 
      GLFWwindow* window = glfwGetCurrentContext();
      return glfwGetWindowAttrib(window, GLFW_FOCUSED); 
	}

private:
  float dirty_scroll_value = 0;
  float current_scroll_value = 0;
  std::unordered_map<int, bool> mKeyStatusMap{};
  std::unordered_map<int, bool> mMouseStatusMap{};
  glm::dvec2 mCursorPosition{};

};