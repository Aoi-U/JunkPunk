#pragma once

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "InputManager.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"

class Renderer
{
public:
	Renderer(std::shared_ptr<InputManager> inputMgr);
	~Renderer() = default;

	void Init(); 
	void Clear(float r, float g, float b, float a); // clears screen and buffers
	void SetCamera(glm::vec3 pos) { cameraPos = pos; } // set camera position
	void SetLight(glm::vec3 pos, glm::vec4 color) { lightPos = pos; lightColor = color; } // set light properties

	void Draw(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader); // main draw function for rendering objects

	void DrawMesh(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Mesh> mesh); // draw function for rendering meshes

	void DrawModel(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Model> modelObj); // draw function for rendering models

private:
	std::shared_ptr<InputManager> inputManager; // not used but maybe useful later idk

	// probably not ideal for renderer to see camera and light info, change to better solution later
	// camera
	glm::vec3 cameraPos{}; 
	// light properties
	glm::vec3 lightPos{}; 
	glm::vec4 lightColor{}; 

};