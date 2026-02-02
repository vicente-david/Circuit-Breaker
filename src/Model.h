#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 col; //replace with normals?
	glm::vec2 tex;
};

struct Texture {
	unsigned int id;
	const char* type;
};

class Model
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	unsigned int VAO;
	Model(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void initModel();
	void Draw();
	
private:
	//void initModel();
	unsigned int VBO, EBO;
};