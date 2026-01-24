#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Game.h"


int main()
{
	glfwInit();
	{
		Game game{};
		game.Run();
	}
	glfwTerminate();

}