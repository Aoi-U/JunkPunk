#include "ft2build.h"
#include FT_FREETYPE_H

#include "Text.h"

Text::Text()
{
  charArial = initFont("assets/fonts/arial/ARIAL.TTF");
}

void Text::initVAO(VAO* vao, VBO* vbo)
{
  vao->Bind();
  vbo->Bind();
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

  vao->LinkAttributes(*vbo, 0, 4, GL_FLOAT, 4 * sizeof(float), (void*)0);

  vao->Unbind();
  vbo->Unbind();
}

std::map<char, Character> Text::initFont(const char* font)
{
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

  std::cout << "Successfully loaded fonts" << std::endl;

  return Characters;
}

