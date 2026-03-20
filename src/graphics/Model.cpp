#include "Model.h"
#include <iostream>


void Model::Draw(GLuint& shaderID) {
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shaderID);
	}
}

std::vector<Mesh> Model::GetMeshes() {
	return meshes;
}

Mesh Model::GetMesh(std::string name) {
	for (auto& mesh : meshes) {
		if (mesh.name == name) {
			return mesh;
		}
	}
	std::cout << "ERROR: Model contains no mesh named " << name << std::endl;
	throw std::runtime_error("Mesh fetch failure");

}

void Model::loadModel(std::string path) {
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ASSIMP error: " << import.GetErrorString() << std::endl;
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode* node, const aiScene* scene) {

	// Process meshes at this node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	// Recurse on children of this node
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}

}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::string name;

	// Process vertex position and texture data
	
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		glm::vec3 posVec = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		vertex.pos = posVec;

		if (mesh->mTextureCoords[0]) {
			// get tex coords if contained in mesh
			// the y axis is flipped when exporting from blender, so we match that
			glm::vec2 texVec = { mesh->mTextureCoords[0][i].x, (1-mesh->mTextureCoords[0][i].y) };
			vertex.tex = texVec;
		}
		else vertex.tex = glm::vec2(0.0f, 0.0f);

		if (mesh->HasNormals()) {
			// Process normals
			glm::vec3 normVec = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			vertex.norm = normVec;
		}
		else vertex.norm = glm::vec3(0.0f);
		

		vertices.push_back(vertex);
	}

	// Process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Process materials (right now only one texture, will have to update this later)
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseTex = loadMatTextures(material, aiTextureType_DIFFUSE, "diffuse");
		textures.insert(textures.end(), diffuseTex.begin(), diffuseTex.end());
	}

	// Get name of mesh
	aiString meshName = mesh->mName;
	if (!meshName.Empty()) {
		name = meshName.C_Str();
	}
	else name = "";
	

	return Mesh(vertices, indices, textures, name);
}

std::vector<Texture> Model::loadMatTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string path = directory.c_str() + std::string("/") + std::string(str.C_Str()); // path of texture relative to model
		Texture tex;
		tex.id = GenerateTexture(path.c_str(), true);
		tex.type = typeName.c_str();
		textures.push_back(tex);
	}
	return textures;
}
