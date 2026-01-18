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

	void Blit();

	void Resize(GLuint width, GLuint height);
	void Cleanup();
private:
	GLuint rbo;
	GLuint fbo;
	GLuint texColorbufferMultisampled;

	GLuint intermediateFbo;
	GLuint screenTexture;

	VAO vao;
	VBO vbo;

	GLuint width;
	GLuint height;
};