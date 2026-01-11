#pragma once

#include <glad/glad.h>

#include "VBO.h"

class VAO
{
public:
	GLuint ID;
	VAO();
	void LinkAttributes(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	void Bind();
	void Unbind();
	void Delete();
};