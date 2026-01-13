#include "Model.h"

Model::Model(const std::string& filePath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
			std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
	}

	ProcessNode(scene->mRootNode, scene);

	if (ProcessMaterials(scene, filePath))
	{
		std::cout << "Textures loaded successfully." << std::endl;
	}
	else
	{
		std::cout << "Some textures failed to load." << std::endl;
	}
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	// process al the node's meshes
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}

	// recursively do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	vertices.reserve(mesh->mNumVertices);

	// get vertex attributes from aiMesh
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex{};

		// positions
		vertex.position = glm::vec3(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		);

		// colors
		if (mesh->HasVertexColors(0))
		{ 
			vertex.color = glm::vec3(
				mesh->mColors[0][i].r,
				mesh->mColors[0][i].g,
				mesh->mColors[0][i].b
			);
		}
		else
		{
			// fill with white
			vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
		}

		// normals
		if (mesh->HasNormals())
		{
			vertex.normal = glm::vec3(
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			);
		}
		else
		{
			vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		// texture coordinates
		if (mesh->mTextureCoords[0]) 
		{
			vertex.texCoord = glm::vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);
		}
		else
		{
			vertex.texCoord = glm::vec2(0.0f, 0.0f);
		}


		vertices.push_back(vertex);
	}

	// get indices from aiMesh
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	Mesh mMesh = Mesh(vertices, indices);
	mMesh.setMaterialIndex(mesh->mMaterialIndex);

	return mMesh;
}

bool Model::ProcessMaterials(const aiScene* scene, const std::string& filePath)
{
	std::string::size_type slashIndex = filePath.find_last_of("/");
	std::string directory;

	if (slashIndex == std::string::npos)
	{
		directory = ".";
	}
	else if (slashIndex == 0)
	{
		directory = "/";
	}
	else
	{
		directory = filePath.substr(0, slashIndex);
	}

	bool success = true;

	// process materials
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* mat = scene->mMaterials[i];

		if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString str;

			if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &str, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string texturePath(str.data);
				if (texturePath.substr(0, 2) == ".\\")
				{
					texturePath = texturePath.substr(2, texturePath.size() - 2);
				}

				std::cout << "Loading texture: " << texturePath << std::endl;
				
				std::string fullPath = directory + "/" + texturePath;

				std::cout << "Full texture path: " << fullPath << std::endl;

				Texture texture(fullPath);

				if (texture.Load())
				{
					textures.push_back(texture);
				}
				else
				{
					std::cout << "Failed to load texture at: " << fullPath << std::endl;
					success = false;
				}
			}
		}
	}
	return success;
}


