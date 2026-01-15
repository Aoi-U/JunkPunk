#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

struct PostProcessVertex
{
	glm::vec2 position;
	glm::vec2 texCoord;
};

class VBO
{
public:
	GLuint ID;
	VBO(std::vector<Vertex>& vertices);
	VBO(std::vector<glm::mat4>& matrices);
	VBO(std::vector<PostProcessVertex>& vertices);

	void Bind();
	void Unbind();
	void Delete();
};