#include "Model.h"
#include <iostream>

Model::Model(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	initModel();
}

void Model::initModel() {
	std::cout << "init model" << std::endl;
}