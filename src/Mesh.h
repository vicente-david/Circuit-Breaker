#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 tex;
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
	void Draw();
	
private:
	void initMesh();
	unsigned int VBO, EBO, VAO;
};