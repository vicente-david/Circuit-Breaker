#include "Mesh.h"
#include "Texture.h"
#include <iostream>
#include <glad/gl.h>


Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	initMesh();
}


void Mesh::initMesh() {
	unsigned int VAO, VBO, EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tex)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VAO

	this->VAO = VAO;
	this->EBO = EBO;
	this->VBO = VBO;
}

void Mesh::Draw() {
	glActiveTexture(GL_TEXTURE0);
	if (textures.size() > 0) {
		glBindTexture(GL_TEXTURE_2D, textures[0].id);
		//glUniform1i(glGetUniformLocation())
	}
	else {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	
}

