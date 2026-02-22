#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "debugUtils/Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cassert>
#include <string>
#include <memory>


class Model {

public:
	// a default constructor is needed for it to be registered in the ecs
	Model() { }
	Model(char* path) {
		loadModel(path);
	}
	void Draw(GLuint& shaderID);
	std::vector<Mesh> GetMesh();
private:
	std::vector<Mesh> meshes;
	std::string directory;
	
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMatTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};
