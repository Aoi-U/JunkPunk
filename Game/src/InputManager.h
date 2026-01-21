#pragma once

#include <functional>
#include<memory>

#include <iostream>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include "glm/glm.hpp"


class Gamepad;

class InputManager
{
public:
	using ResizeCallback = std::function<void(int width, int height)>;

	InputManager(ResizeCallback resizeCallback);

  ~InputManager();

	bool IsKeyboardButtonDown(int keyboardButton) const;

	bool IsMouseButtonDown(int mouseButton) const;

	glm::dvec2 const& CursorPosition() const;

	int ScrollValueChanged();

  void keyCallback(int key, int scancode, int action, int mods);

  void windowSizeCallback(int width, int height);

  void mouseButtonCallback(int button, int action, int mods);

  void cursorPosCallback(double xpos, double ypos);

  void scrollCallback(double xoffset, double yoffset);

  bool IsWindowFocused() const { 
      GLFWwindow* window = glfwGetCurrentContext();
      return glfwGetWindowAttrib(window, GLFW_FOCUSED); 
	}

  // add support for controller
  bool IsControllerConnected() const;
  float GetLStickTurnValue() const;
  float GetRStickTurnValueX() const;
	float GetRStickTurnValueY() const;
  float GetThrottleValue() const;
  float GetBrakeValue() const;
  bool IsButtonPressed(int button) const; // pressed: returns true if button is held down
  bool IsButtonDown(int button) const; // down: returns true only on the frame the button is first pressed
  
private:
  float dirty_scroll_value = 0;
  float current_scroll_value = 0;
  std::unordered_map<int, bool> mKeyStatusMap{};
  std::unordered_map<int, bool> mMouseStatusMap{};
  glm::dvec2 mCursorPosition{};
  ResizeCallback mResizeCallback;

  std::unique_ptr<Gamepad> gamepad;
};