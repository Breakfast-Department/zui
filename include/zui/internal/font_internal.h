#ifndef ZUI_FONT_INTERNAL_H
#define ZUI_FONT_INTERNAL_H

#include <zui/font.h>
#include <zui/internal/renderer.h>
#include <stdbool.h>

#define ZUI_FONT_ATLAS_SIZE 512
#define ZUI_FONT_FIRST_CHAR 32
#define ZUI_FONT_NUM_CHARS 96

typedef struct ZuiGlyph {
    float x0, y0, x1, y1;
    float u0, v0, u1, v1;
    float advance;
} ZuiGlyph;

struct ZuiFont {
    ZuiTexture atlas;
    ZuiGlyph glyphs[ZUI_FONT_NUM_CHARS];
    float size;
    float ascent;
    float descent;
    float line_height;
    unsigned char *font_data;
    bool owns_data;
};

void zui_font_render_text(ZuiFont *font, ZuiRenderer *renderer,
                          float x, float y, const char *text, ZuiColor color);

#endif
