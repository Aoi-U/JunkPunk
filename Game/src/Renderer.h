#pragma once

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "InputManager.h"
#include "Shader.h"
#include "Mesh.h"

class Renderer
{
public:
	Renderer(std::shared_ptr<InputManager> inputMgr);
	~Renderer() = default;

	void Init(); 
	void Clear(float r, float g, float b, float a); // clears screen and buffers

	void Draw(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader); // main draw function for rendering objects

	void DrawMesh(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Mesh> mesh); // draw function for rendering meshes



private:
	std::shared_ptr<InputManager> inputManager; // not used but maybe useful later idk
};