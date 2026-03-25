#include <glad/gl.h>
#include<glm/glm.hpp>
#include <map>
#include "ShaderProgram.h"
#include "../ui/UISystemComponents.h"

struct Character {
	unsigned int textID; // ID handle of the texture
	glm::ivec2 size; // size of glyph
	glm::ivec2 bearing; // Offset from baseline
	unsigned int Advance; // Offset to next glyph
};

std::map<char, Character> initFont(const char* font);

void calcPosition(textPositions& positions, float& x, float& y); // x represents the left starting point, y represents the bottom starting point

void RenderText(GLuint sID, unsigned int VAO, unsigned int VBO, std::string text, textPositions positions, float scale, glm::vec3 color, std::map<char, Character> Characters);