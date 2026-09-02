#ifndef ZUI_FONT_H
#define ZUI_FONT_H

#include <stdbool.h>

typedef struct ZuiFont ZuiFont;

ZuiFont *zui_font_load(const char *path, float size);
ZuiFont *zui_font_load_memory(const unsigned char *data, int size, float font_size);
void zui_font_destroy(ZuiFont *font);

float zui_font_text_width(ZuiFont *font, const char *text);
float zui_font_line_height(ZuiFont *font);
float zui_font_ascent(ZuiFont *font);
float zui_font_descent(ZuiFont *font);

#endif
