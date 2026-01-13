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

void Renderer::DrawMesh(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, Mesh& mesh)
{
	shader->use();
	shader->setMat4("u_model", model);
	shader->setMat4("u_projView", projView);
	shader->setVec3("u_cameraPos", &cameraPos.x);

	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	// bind each texture for the model
	for (unsigned int i = 0; i < mesh.getTextures().size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding

		std::string number;
		std::string name = mesh.getTextures()[i].getType();

		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++); // transfer unsigned int to string
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // transfer unsigned int to string
		else if (name == "texture_height")
			number = std::to_string(heightNr++); // transfer unsigned int to string

		//shader->setInt(("material." + name + number).c_str(), i);
		shader->setInt((name + number).c_str(), i); // set the texture unit in the shader
		mesh.getTextures()[i].Bind(GL_TEXTURE0 + i);
	}

	mesh.BindVao();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);

	mesh.UnbindVao();
	glActiveTexture(GL_TEXTURE0);
}

void Renderer::DrawModel(const glm::mat4& model, const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Model> modelObj)
{
	// draw each mesh in the model
	for (Mesh& mesh : modelObj->getMeshes())
	{
		DrawMesh(model, projView, shader, mesh);
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

