#pragma once

#include <vector>
#include <glad/glad.h>

class EBO
{
public:
	GLuint ID;
	EBO(std::vector<GLuint>& indices);
	void Bind();
	void Unbind();
	void Delete();
};