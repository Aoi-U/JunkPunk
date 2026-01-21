#include "InputManager.h"
#include "../Gamepad.h"

//#include <utility>

InputManager::InputManager(ResizeCallback resizeCallback)
  : mResizeCallback(std::move(resizeCallback)), dirty_scroll_value(0), current_scroll_value(0)
{
	gamepad = std::make_unique<Gamepad>(1);
}

InputManager::~InputManager() = default;

void InputManager::keyCallback(int const key, int const scancode, int const action, int const mods)
{
  if (action == GLFW_PRESS)
  {
    mKeyStatusMap[key] = true;
  }
  else if (action == GLFW_RELEASE)
  {
    mKeyStatusMap[key] = false;
  }
}

void InputManager::windowSizeCallback(int const width, int const height)
{
  if (mResizeCallback != nullptr)
  {
    mResizeCallback(width, height);
  }
}

void InputManager::mouseButtonCallback(int const button, int const action, int mods)
{
  mMouseStatusMap[button] = action;
}

void InputManager::cursorPosCallback(double const xpos, double const ypos)
{
  mCursorPosition.x = xpos;
  mCursorPosition.y = ypos;
}

void InputManager::scrollCallback(double const xoffset, double const yoffset)
{
    dirty_scroll_value += yoffset;
}


bool InputManager::IsKeyboardButtonDown(int const keyboardButton) const
{
  bool isButtonDown = false;
  auto const findResult = mKeyStatusMap.find(keyboardButton);
  if (findResult != mKeyStatusMap.end())
  {
    isButtonDown = findResult->second;
  }
  return isButtonDown;
}

bool InputManager::IsMouseButtonDown(int const mouseButton) const
{
  bool isButtonDown = false;
  auto const findResult = mMouseStatusMap.find(mouseButton);
  if (findResult != mMouseStatusMap.end())
  {
    isButtonDown = findResult->second;
  }
  return isButtonDown;
}

glm::dvec2 const& InputManager::CursorPosition() const
{
  return mCursorPosition;
}

int InputManager::ScrollValueChanged()
{
    int changed = current_scroll_value - dirty_scroll_value;
    current_scroll_value = dirty_scroll_value;
    return changed;
}

bool InputManager::IsControllerConnected() const
{
  return gamepad->Connected();
}

float InputManager::GetLStickTurnValue() const
{
  return gamepad->LStick_InDeadzone() ? 0.0f : -gamepad->LeftStick_X();
}

float InputManager::GetRStickTurnValueX() const
{
  return gamepad->RStick_InDeadzone() ? 0.0f : -gamepad->RightStick_X();
}

float InputManager::GetRStickTurnValueY() const
{
  return gamepad->RStick_InDeadzone() ? 0.0f : -gamepad->RightStick_Y();
}

float InputManager::GetThrottleValue() const
{
  return gamepad->RightTrigger();
}

float InputManager::GetBrakeValue() const
{
  return gamepad->LeftTrigger();
}

bool InputManager::IsButtonPressed(int button) const
{
  return gamepad->GetButtonPressed(button);
}

bool InputManager::IsButtonDown(int button) const
{
  return gamepad->GetButtonDown(button);
}
