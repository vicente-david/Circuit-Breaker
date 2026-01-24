#include<glm/glm.hpp>
#include <map>
#include "Shader.h"

struct Character {
	unsigned int textID; // ID handle of the texture
	glm::ivec2 size; // size of glyph
	glm::ivec2 bearing; // Offset from baseline
	unsigned int Advance; // Offset to next glyph
};

std::map<char, Character> initFont(const char* font);

void RenderText(Shader& s, unsigned int VAO, unsigned int VBO, std::string text, float x, float y, float scale, glm::vec3 color, std::map<char, Character> Characters);