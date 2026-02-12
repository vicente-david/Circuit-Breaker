#pragma once
#include "Mesh.h"
#include "Texture.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <memory>


class Model {

public:
	Model(char* path) {
		loadModel(path);
	}
	void Draw(GLuint& shaderID);
private:
	std::vector<Mesh> meshes;
	std::string directory;
	
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMatTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};