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
	Model(const std::string& filePath);

	const std::vector<Mesh>& getMeshes() { return meshes; }

private:
	std::vector<Mesh> meshes;

	void ProcessNode(aiNode* node, const aiScene* scene); // processes a node in the scene
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene); // creates a mesh from an aiMesh
};

