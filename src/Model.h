#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	//glm::vec3 col; //replace with normals?
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

	Model(char* path);
	Model(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw();
	
private:
	void loadModel(std::string path);
	void initModel();
	unsigned int VBO, EBO, VAO;
	std::string directory;
};