#include "Model.h"

// helper function to convert aiMatrix4x4 to glm::mat4
// required for models with node transformations
glm::mat4 aiMatrix4v4ToGlmMat4Helper(const aiMatrix4x4& parent)
{
	glm::mat4 result;
	result[0][0] = parent.a1; result[1][0] = parent.a2; result[2][0] = parent.a3; result[3][0] = parent.a4;
	result[0][1] = parent.b1; result[1][1] = parent.b2; result[2][1] = parent.b3; result[3][1] = parent.b4;
	result[0][2] = parent.c1; result[1][2] = parent.c2; result[2][2] = parent.c3; result[3][2] = parent.c4;
	result[0][3] = parent.d1; result[1][3] = parent.d2; result[2][3] = parent.d3; result[3][3] = parent.d4;
	return result;
}

Model::Model(std::string filePath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
			std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
	}
	directory = filePath.substr(0, filePath.find_last_of('/'));
	std::cout << "directory: " << directory << std::endl;


	ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f));
}

void Model::Cleanup()
{
	for (auto& mesh : meshes)
	{
		mesh.Cleanup();
	}

	for (auto& texture : texturesLoaded)
	{
		texture.Delete();
	}
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform)
{
	glm::mat4 nodeTransform = aiMatrix4v4ToGlmMat4Helper(node->mTransformation);
	glm::mat4 globalTransform = parentTransform * nodeTransform;

	// process al the node's meshes
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene, globalTransform));
	}

	// recursively do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, globalTransform);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	vertices.reserve(mesh->mNumVertices);

	glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(transform)));

	// get vertex attributes from aiMesh
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex{};

		// positions
		glm::vec4 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		position.w = 1.0f;

		position = transform * position;

		vertex.position = glm::vec3(position);

		glm::vec3 vector;
		// colors
		if (mesh->HasVertexColors(0))
		{ 
			vector.x = mesh->mColors[0][i].r;
			vector.y = mesh->mColors[0][i].g;
			vector.z = mesh->mColors[0][i].b;
			
			vertex.color = vector;
		}
		else
		{
			// fill with white
			vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
		}

		// normals
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;

			vertex.normal = normalMat * vector;
		}
		else
		{
			vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		// texture coordinates
		if (mesh->mTextureCoords[0]) 
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoord = vec;
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

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	/*std::cout << "material texture counts" << std::endl;
	for (int i = aiTextureType_NONE; i <= aiTextureType_UNKNOWN; i++) {
		aiTextureType type = static_cast<aiTextureType>(i);
		unsigned int count = material->GetTextureCount(type);
		if (count > 0) {
			std::cout << "  Type " << i << ": " << count << " textures" << std::endl;
		}
	}*/

	std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

	std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	
	
	Mesh mMesh = Mesh(vertices, indices, textures);
	mMesh.SetupMesh();

	return mMesh;
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
	std::vector<Texture> textures;
	
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (unsigned int j = 0; j < texturesLoaded.size(); j++)
		{
			if (std::strcmp(texturesLoaded[j].getPath().data(), str.C_Str()) == 0)
			{
				textures.push_back(texturesLoaded[j]);
				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			Texture texture(str.C_Str(), typeName);
			texture.Load(directory);
			textures.push_back(texture);
			texturesLoaded.push_back(texture);		
		}
	}

	return textures;
}


