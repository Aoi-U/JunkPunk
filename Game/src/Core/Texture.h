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
		Texture(const std::string& path, const std::string& type);
		bool Load(const std::string& directory); // loads the texture from file
		void Bind(GLenum unit) const;
		void Unbind();
		void Delete();

		const std::string& getPath() { return path; }
		const std::string& getType() { return type; }
		GLuint getID() const { return ID; }
private:
	std::string path;
	std::string type;
	GLuint ID{};
};