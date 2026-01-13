#pragma once

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include "stb/stb_image.h"

class Texture
{
	public:
		Texture(const std::string& path);
		bool Load(); // loads the texture from file
		void Bind(GLenum unit) const;
		void Unbind();
		void Delete();
		GLuint getID() const { return ID; }
private:
	const std::string path;
	GLuint ID;
};