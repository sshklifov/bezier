#include <ft2build.h>
#include FT_FREETYPE_H

#include "Common.h"
#include <vector>

struct Character
{
    int width;
    int height;
    std::vector<unsigned char> buffer;
};
std::vector<Character> chars;

void InitRenderText()
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        fprintf(stderr, "freetype failed to init\n");
        exit(1);
    }

    FT_Face face;
    const char* font = "fonts/Arimo-Regular.ttf";
    if (FT_New_Face(ft, font, 0, &face))
    {
        fprintf(stderr, "freetype failed to load %s\n", font);
        exit(1);
    }
    FT_Set_Pixel_Sizes(face, 0, 13);

    chars.resize(10);
    for (int c = '0'; c <= '9'; ++c)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            fprintf(stderr, "freetype failed to load glyph\n");
            exit(1);
        }
        Character& ccur = chars[c - '0'];
        ccur.width = face->glyph->bitmap.width;
        ccur.height = face->glyph->bitmap.rows;
        ccur.buffer.resize(ccur.width * ccur.height);
        for (int row = 0; row < ccur.height; ++row)
        {
            memcpy(ccur.buffer.data() + ccur.width * row,
                   face->glyph->bitmap.buffer +
                       ccur.width * (ccur.height - 1 - row),
                   ccur.width);
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void RenderText(glm::vec3* buf, int idx, glm::ivec2 pos)
{
    const glm::vec3 color = black;
    const Character& dig = chars.at(idx);

    int startx = pos.x - dig.width / 2;
    int starty = pos.y - dig.height / 2;
    int bmapIdx = 0;
    for (int yoff = 0; yoff < dig.height; ++yoff)
    {
        int fbIdx = (yoff + starty) * WIDTH + startx;
        for (int xoff = 0; xoff < dig.width; ++xoff)
        {
            float t = dig.buffer[bmapIdx] / 255.f;
            buf[fbIdx] = glm::mix(buf[fbIdx], color, t);
            ++bmapIdx;
            ++fbIdx;
        }
    }
}
