#pragma once

#include <iostream>

#include "VBO.h"
#include "VAO.h"

class PostProcessor
{
public:
	PostProcessor(GLuint width, GLuint height);

	void BindVAO();
	void BindTexture();
	void BindFBO();
	void Unbind();

	void Resize(GLuint width, GLuint height);
	void Cleanup();
private:
	GLuint rbo;
	GLuint fbo;
	GLuint texColorbuffer;

	VAO vao;
	VBO vbo;
};