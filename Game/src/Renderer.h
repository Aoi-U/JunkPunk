#pragma once

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "InputManager.h"
#include "Shader.h"
#include "Mesh.h"
#include "Skybox.h"
#include "Entity.h"
#include "PostProcessor.h"

class Renderer
{
public:
	Renderer(std::shared_ptr<InputManager> inputMgr);
	~Renderer() = default;

	void Init(); 
	void Clear(float r, float g, float b, float a); // clears screen and buffers
	void SetCamera(glm::vec3 pos) { cameraPos = pos; } // set camera position

	void DrawEntity(const glm::mat4& projView, std::shared_ptr<Shader> shader, Entity& entity); // draw function for rendering entities

	void DrawEntityInstanced(const glm::mat4& projView, std::shared_ptr<Shader> shader, Entity& entity, const std::vector<glm::mat4>& translations); // draw function for rendering instanced entities

	void DrawSkybox(const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Skybox> skybox); // draw function for rendering skybox

	void DrawQuad(std::shared_ptr<Shader> shader, std::shared_ptr<PostProcessor> postProcessor); // draw function for rendering a quad (used for post processing)

private:
	std::shared_ptr<InputManager> inputManager; // not used but maybe useful later idk

	// probably not ideal for renderer to see camera, change to better solution later
	glm::vec3 cameraPos{}; 

	void DrawMesh(Mesh& mesh, const glm::mat4& projView, std::shared_ptr<Shader> shader); // draw function for rendering meshes
	void BindTextures(Mesh& mesh, std::shared_ptr<Shader> shader); // bind textures for a mesh
};