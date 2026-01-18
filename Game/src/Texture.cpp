#include "Texture.h"

Texture::Texture(const std::string& path)
	: path(path)
{
	
}

Texture::Texture(const std::string& path, const std::string& type)
	: path(path), type(type)
{
}

bool Texture::Load(const std::string& directory)
{
	glGenTextures(1, &ID);

	int width, height, nrChannels;
	//stbi_set_flip_vertically_on_load(true);
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		GLenum imageFormat{};
		if (nrChannels == 1)
			imageFormat = GL_RED;
		else if (nrChannels == 3)
			imageFormat = GL_RGB;
		else if (nrChannels == 4)
			imageFormat = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data);
		//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, imageFormat, width, height, GL_TRUE);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		std::cout << "Successfully loaded texture at path: " << path << std::endl;
	}
	else
	{
		std::cout << "Failed to load texture at path: " << path << std::endl;
		stbi_image_free(data);
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void Texture::Bind(GLenum unit) const
{
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}
