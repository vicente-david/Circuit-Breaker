#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 col;
	glm::vec2 tex;
};

class Model
{
public:
	std::vector<Vertex> verts;
	glm::mat4 modelMatrix;
};