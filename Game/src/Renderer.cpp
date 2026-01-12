#include "Renderer.h"

Renderer::Renderer(std::shared_ptr<InputManager> inputMgr)
	: inputManager(inputMgr)
{
	glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
}

//Renderer::~Renderer()
//{
//
//}

void Renderer::Init()
{
	
}

void Renderer::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader)
{
	shader->use();
	shader->setMat4("u_model", &model[0][0]);
	shader->setMat4("u_projView", &projView[0][0]);
	shader->setVec3("u_cameraPos", &cameraPos.x);
	shader->setVec3("u_lightPos", &lightPos.x);
	shader->setVec4("u_lightColor", &(lightColor.r));

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::DrawMesh(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Mesh> mesh)
{
	shader->use();
	shader->setMat4("u_model", &model[0][0]);
	shader->setMat4("u_projView", &projView[0][0]);
	shader->setVec3("u_cameraPos", &cameraPos.x);
	shader->setVec3("u_lightPos", &lightPos.x);
	shader->setVec4("u_lightColor", &lightColor.r);

	mesh->BindVao();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndices().size()), GL_UNSIGNED_INT, 0);
}

void Renderer::DrawModel(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Model> modelObj)
{
	// support for textures not added yet, just draws meshes with hard coded color

	shader->use();
	shader->setMat4("u_model", &model[0][0]);
	shader->setMat4("u_projView", &projView[0][0]);
	shader->setVec3("u_cameraPos", &cameraPos.x);
	shader->setVec3("u_lightPos", &(lightPos.x));
	shader->setVec4("u_lightColor", &(lightColor.r));

	// draw each mesh in the model
	for (const Mesh& mesh : modelObj->getMeshes())
	{
		mesh.BindVao();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);
	}
}

