#pragma once

#include <string>
#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/glm.hpp"
#include "Mesh.h"

class Model
{
public:
	// loads and processes a model from a file
	Model(std::string filePath);

	void Cleanup();

	std::vector<Mesh>& getMeshes() { return meshes; }
	Texture& getTexture(unsigned int index) { return texturesLoaded[index]; }

private:
	std::string directory;
	std::vector<Mesh> meshes;
	std::vector<Texture> texturesLoaded;

	void ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform = glm::mat4(1.0f)); // processes a node in the scene
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform); // creates a mesh from an aiMesh

	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
};

