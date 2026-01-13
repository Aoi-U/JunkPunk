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
	Model(const std::string& filePath);

	const std::vector<Mesh>& getMeshes() { return meshes; }
	const Texture& getTexture(unsigned int index) const { return textures[index]; }

private:
	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
	

	void ProcessNode(aiNode* node, const aiScene* scene); // processes a node in the scene
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene); // creates a mesh from an aiMesh
	bool ProcessMaterials(const aiScene* scene, const std::string& filePath);
};

