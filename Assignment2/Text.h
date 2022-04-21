#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>

#include "shader.h"

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

class Text
{
public:
	Text();
	~Text();

	void Render(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
    
private:
    std::map<GLchar, Character> Characters;
    GLuint VAO, VBO;
    Shader shader;
    Shader shader3D;
};

