#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include <zui/internal/font_internal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *load_file(const char *path, int *size)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len < 0) {
        fclose(f);
        return NULL;
    }
    *size = (int)len;
    fseek(f, 0, SEEK_SET);

    unsigned char *data = malloc((size_t)len);
    if (!data) {
        fclose(f);
        return NULL;
    }

    if (fread(data, 1, (size_t)len, f) != (size_t)len) {
        free(data);
        fclose(f);
        return NULL;
    }

    fclose(f);
    return data;
}

bool init_font(ZuiFont *font, const unsigned char *font_data, float size)
{
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, font_data, 0)) {
        return false;
    }

    float scale = stbtt_ScaleForPixelHeight(&info, size);

    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
    font->ascent = (float)ascent * scale;
    font->descent = (float)descent * scale;
    font->line_height = (float)(ascent - descent + line_gap) * scale;
    font->size = size;

    int atlas_w = ZUI_FONT_ATLAS_SIZE;
    int atlas_h = ZUI_FONT_ATLAS_SIZE;
    unsigned char *atlas_bitmap = calloc(1, (size_t)(atlas_w * atlas_h));
    if (!atlas_bitmap) return false;

    int pen_x = 1, pen_y = 1;
    int row_height = 0;

    for (int i = 0; i < ZUI_FONT_NUM_CHARS; i++) {
        int c = ZUI_FONT_FIRST_CHAR + i;

        int w, h, xoff, yoff;
        unsigned char *bitmap = stbtt_GetCodepointBitmap(
            &info, 0, scale, c, &w, &h, &xoff, &yoff);

        if (pen_x + w + 1 >= atlas_w) {
            pen_x = 1;
            pen_y += row_height + 1;
            row_height = 0;
        }

        if (pen_y + h + 1 >= atlas_h) {
            if (bitmap) stbtt_FreeBitmap(bitmap, NULL);
            free(atlas_bitmap);
            return false;
        }

        if (bitmap) {
            for (int y = 0; y < h; y++) {
                memcpy(&atlas_bitmap[(pen_y + y) * atlas_w + pen_x],
                       &bitmap[y * w], (size_t)w);
            }
            stbtt_FreeBitmap(bitmap, NULL);
        }

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&info, c, &advance, &lsb);

        ZuiGlyph *g = &font->glyphs[i];
        g->x0 = (float)xoff;
        g->y0 = (float)yoff;
        g->x1 = (float)(xoff + w);
        g->y1 = (float)(yoff + h);
        g->u0 = (float)pen_x / (float)atlas_w;
        g->v0 = (float)pen_y / (float)atlas_h;
        g->u1 = (float)(pen_x + w) / (float)atlas_w;
        g->v1 = (float)(pen_y + h) / (float)atlas_h;
        g->advance = (float)advance * scale;

        pen_x += w + 1;
        if (h > row_height) row_height = h;
    }

    unsigned char *rgba = malloc((size_t)(atlas_w * atlas_h * 4));
    if (!rgba) {
        free(atlas_bitmap);
        return false;
    }

    for (int i = 0; i < atlas_w * atlas_h; i++) {
        rgba[i * 4 + 0] = 255;
        rgba[i * 4 + 1] = 255;
        rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = atlas_bitmap[i];
    }

    font->atlas = zui_texture_create(rgba, atlas_w, atlas_h);
    free(rgba);
    free(atlas_bitmap);

    return font->atlas.id != 0;
}

ZuiFont *zui_font_load(const char *path, float size)
{
    int data_size;
    unsigned char *data = load_file(path, &data_size);
    if (!data) {
        fprintf(stderr, "ZUI: Failed to load font: %s\n", path);
        return NULL;
    }

    ZuiFont *font = calloc(1, sizeof(ZuiFont));
    if (!font) {
        free(data);
        return NULL;
    }

    font->font_data = data;
    font->owns_data = true;

    if (!init_font(font, data, size)) {
        free(data);
        free(font);
        return NULL;
    }

    return font;
}

ZuiFont *zui_font_load_memory(const unsigned char *data, int size, float font_size)
{
    (void)size;

    ZuiFont *font = calloc(1, sizeof(ZuiFont));
    if (!font) return NULL;

    font->font_data = (unsigned char *)data;
    font->owns_data = false;

    if (!init_font(font, data, font_size)) {
        free(font);
        return NULL;
    }

    return font;
}

void zui_font_destroy(ZuiFont *font)
{
    if (!font) return;

    zui_texture_destroy(&font->atlas);
    if (font->owns_data && font->font_data) {
        free(font->font_data);
    }
    free(font);
}

float zui_font_text_width(ZuiFont *font, const char *text)
{
    if (!font || !text) return 0;

    float max_width = 0;
    float line_width = 0;

    while (*text) {
        int c = (unsigned char)*text;
        if (c == '\n') {
            if (line_width > max_width) max_width = line_width;
            line_width = 0;
        } else if (c >= ZUI_FONT_FIRST_CHAR && c < ZUI_FONT_FIRST_CHAR + ZUI_FONT_NUM_CHARS) {
            line_width += font->glyphs[c - ZUI_FONT_FIRST_CHAR].advance;
        }
        text++;
    }

    if (line_width > max_width) max_width = line_width;
    return max_width;
}

float zui_font_text_height(ZuiFont *font, const char *text)
{
    if (!font || !text) return 0;

    int lines = 1;
    while (*text) {
        if (*text == '\n') lines++;
        text++;
    }
    return font->line_height * (float)lines;
}

float zui_font_line_height(ZuiFont *font)
{
    return font ? font->line_height : 0;
}

float zui_font_ascent(ZuiFont *font)
{
    return font ? font->ascent : 0;
}

float zui_font_descent(ZuiFont *font)
{
    return font ? font->descent : 0;
}

void zui_font_render_text(ZuiFont *font, ZuiRenderer *renderer,
                          float x, float y, const char *text, ZuiColor color)
{
    if (!font || !renderer || !text) return;

    float cursor_x = x;
    float baseline_y = y + font->ascent;

    while (*text) {
        int c = (unsigned char)*text;
        if (c == '\n') {
            cursor_x = x;
            baseline_y += font->line_height;
        } else if (c >= ZUI_FONT_FIRST_CHAR && c < ZUI_FONT_FIRST_CHAR + ZUI_FONT_NUM_CHARS) {
            ZuiGlyph *g = &font->glyphs[c - ZUI_FONT_FIRST_CHAR];

            float gx = cursor_x + g->x0;
            float gy = baseline_y + g->y0;
            float gw = g->x1 - g->x0;
            float gh = g->y1 - g->y0;

            if (gw > 0 && gh > 0) {
                zui_renderer_draw_glyph(renderer, &font->atlas,
                    ZUI_RECT(gx, gy, gw, gh),
                    g->u0, g->v0, g->u1, g->v1, color);
            }

            cursor_x += g->advance;
        }
        text++;
    }
}
