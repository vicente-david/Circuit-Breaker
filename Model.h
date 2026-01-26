#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 col;
};

class Model
{
public:
	Vertex* verts;
	glm::mat4 modelMatrix;
};