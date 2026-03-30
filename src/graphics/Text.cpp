#include<ft2build.h>
#include FT_FREETYPE_H
#include "Text.h"
#include <iostream>

std::map<char, Character> initFont(const char* font) {
    std::map<char, Character> Characters;
    FT_Library ft;
    if (FT_Init_FreeType(&ft))

    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return Characters;
    }

    FT_Face face;
    if (FT_New_Face(ft, font, 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return Characters;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //Initialize characters
    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return Characters;
}

// top left are default, so only change those that are different
// totalwidth is width of text
// totalheight is height of text
void calcPositions(textPositions& positions, float& x, float& y, int totalWidth, int totalHeight) {
    float containerWidth = positions.rightPx - positions.leftPx;
    float containerHeight = positions.bottomPx - positions.topPx;

    switch (positions.textAlignX) {
    case(CENTER):
        x = positions.leftPx + 0.5f * containerWidth - 0.5f*totalWidth;
        break;
    case(RIGHT):
        x = positions.rightPx - totalWidth;
        break;
    }

    switch (positions.textAlignY) {
    case(CENTER):
        y = positions.topPx + containerHeight * 0.5f - 0.5f*totalHeight;
        break;
    case(BOTTOM):
        y = positions.bottomPx - totalHeight;
        break;
    }
}

void RenderText(GLuint sID, unsigned int VAO, unsigned int VBO, std::string text, textPositions positions, float scale, glm::vec3 color, std::map<char, Character> Characters)
{
    float x = positions.leftPx;
    float y = positions.topPx;
    // activate corresponding render state	
    //s.use();
    glUniform3f(glGetUniformLocation(sID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // calc width and height of string
    // only necessary for right align and center align
    std::string::const_iterator c1;
    int totalWidth = 0;
    int maxHeight = 0;
    int maxBearing = 0;
    for (c1 = text.begin(); c1 != text.end(); c1++) {
        Character ch = Characters[*c1];
        totalWidth += (ch.Advance >> 6)*scale;
        maxHeight = glm::max(maxHeight, ch.size.y);
        maxBearing = glm::max(maxBearing, ch.bearing.y);
    }
    
    calcPositions(positions, x, y, totalWidth, maxHeight*scale);
    

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y + (maxBearing - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos,     ypos,       0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 0.0f },

            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}