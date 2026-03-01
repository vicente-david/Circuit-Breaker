#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>


struct Vertex
{
	glm::vec3 pos;
	glm::vec2 tex;
	glm::vec3 norm;
};

struct Texture {
	unsigned int id;
	const char* type;
};

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(GLuint& shaderID);
	std::pair<glm::vec3, glm::vec3> GetBounds();
	
private:
	void initMesh();
	unsigned int VBO, EBO, VAO;

};