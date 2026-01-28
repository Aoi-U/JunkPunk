#pragma once

#include <vector>
#include <glad/glad.h>

class EBO
{
public:
	GLuint ID;
	EBO() = default;
	EBO(std::vector<GLuint>& indices);
	EBO(unsigned int indices[], int size); // quad ebo
	

	void Bind();
	void Unbind();
	void Delete();
};