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
	shader->setMat4("u_model", model);
	shader->setMat4("u_projView", projView);
	shader->setVec3("u_cameraPos", &cameraPos.x);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::DrawMesh(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Mesh> mesh)
{
	shader->use();
	shader->setMat4("u_model", model);
	shader->setMat4("u_projView", projView);
	shader->setVec3("u_cameraPos", &cameraPos.x);

	// mesh has no texture so just draw with default color
	shader->setInt("u_useTexture", 0);
	
	mesh->BindVao();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndices().size()), GL_UNSIGNED_INT, 0);
}

void Renderer::DrawModel(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Model> modelObj)
{
	// support for textures not added yet, just draws meshes with hard coded color

	shader->use();
	shader->setMat4("u_model", model);
	shader->setMat4("u_projView", projView);
	shader->setVec3("u_cameraPos", &cameraPos.x);

	
	// draw each mesh in the model
	for (const Mesh& mesh : modelObj->getMeshes())
	{
		mesh.BindVao();
		shader->setInt("u_useTexture", 1);

		unsigned int materialIndex = mesh.getMaterialIndex();

		const Texture& texture = modelObj->getTexture(materialIndex);
		texture.Bind(GL_TEXTURE0);
		shader->setInt("u_tex0", 0);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);
	}
	
}

void Renderer::DrawSkybox(const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Skybox> skybox)
{
	glDepthFunc(GL_LEQUAL); // change depth function for skybox

	shader->use();
	shader->setMat4("u_projView", projView);


	// bind skybox VAO and cubemap texture
	skybox->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetCubemapTexture());
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	skybox->Unbind();

	glDepthFunc(GL_LESS); // reset depth function

}

