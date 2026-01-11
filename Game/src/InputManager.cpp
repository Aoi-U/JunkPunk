#include "InputManager.h"

//#include <utility>

InputManager::InputManager(ResizeCallback resizeCallback)
  : mResizeCallback(std::move(resizeCallback))
{
}

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
