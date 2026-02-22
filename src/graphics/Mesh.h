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
	
private:
	void initMesh();
	void initShadowMap();
	unsigned int VBO, EBO, VAO;

	// Depth map variables for shadows
	unsigned int SHADOW_W = 1024, SHADOW_H = 1024; //resolution
	unsigned int depthMap;
	unsigned int depthFBO;
};