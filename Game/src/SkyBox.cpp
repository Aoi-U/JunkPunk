#include "SkyBox.h"

Skybox::Skybox()
{
	std::vector<Vertex> skyboxVertices = {
	{{-1.0f, -1.0f,  1.0f}, {}, {}, {}},
	{{ 1.0f, -1.0f,  1.0f}, {}, {}, {}},
	{{ 1.0f, -1.0f, -1.0f}, {}, {}, {}},
	{{-1.0f, -1.0f, -1.0f}, {}, {}, {}},
	{{-1.0f,  1.0f,  1.0f}, {}, {}, {}},
	{{ 1.0f,  1.0f,  1.0f}, {}, {}, {}},
	{{ 1.0f,  1.0f, -1.0f}, {}, {}, {}},
	{{-1.0f,  1.0f, -1.0f}, {}, {}, {}}
	};

	std::vector<GLuint> skyboxIndices = {
		// right
		1, 5, 6,
		6, 2, 1,
		// left
		0, 3, 7,
		7, 4, 0,
		// top
		4, 7, 6,
		6, 5, 4,
		// bottom
		0, 1, 2,
		2, 3, 0,
		// back
		0, 4, 5,
		5, 1, 0,
		// front
		3, 2, 6,
		6, 7, 3
	};

	vao = std::make_unique<VAO>();
	vao->Bind();

	/*VBO vbo(skyboxVertices);
	EBO ebo(skyboxIndices);*/
	vbo = std::make_unique<VBO>(skyboxVertices);
	ebo = std::make_unique<EBO>(skyboxIndices);

	vbo->Bind();
	ebo->Bind();
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind to prevent accidental modification
	vao->Unbind();
	vbo->Unbind();
	ebo->Unbind();
}

void Skybox::Init()
{
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (size_t i = 0; i < faces.size(); i++)
	{
		int width, height, nrChannels;
		
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load skybox texture at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
}

void Skybox::Bind()
{
	vao->Bind();
}

void Skybox::Unbind()
{
	vao->Unbind();
}
void Skybox::Delete()
{
	vao->Delete();
}
