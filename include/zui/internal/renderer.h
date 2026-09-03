#ifndef ZUI_RENDERER_H
#define ZUI_RENDERER_H

#include <glad/glad.h>
#include <stdbool.h>
#include <stdint.h>
#include <zui/color.h>

#define ZUI_MAX_CLIP_STACK 16

typedef struct ZuiRect {
    float x, y, w, h;
} ZuiRect;

typedef struct ZuiClipState {
    ZuiRect rect;
    float radius;
} ZuiClipState;

typedef struct ZuiTexture {
    GLuint id;
    int width;
    int height;
} ZuiTexture;

typedef struct ZuiWindowStyle {
    ZuiColor bg_color;
    ZuiColor border_color;
    ZuiColor titlebar_color;
    float border_width;
    float radius_tl;
    float radius_tr;
    float radius_br;
    float radius_bl;
    float titlebar_height;
    float curve_intensity;
    bool active;
} ZuiWindowStyle;

typedef struct ZuiRenderer {
    GLuint rect_shader;
    GLuint rect_vao;
    GLuint rect_vbo;

    GLuint tex_shader;
    GLuint tex_vao;
    GLuint tex_vbo;

    GLuint glyph_shader;

    GLuint window_shader;
    GLuint window_vao;
    GLuint window_vbo;

    int viewport_width;
    int viewport_height;

    ZuiClipState clip_stack[ZUI_MAX_CLIP_STACK];
    int clip_stack_top;
} ZuiRenderer;

bool zui_renderer_init(ZuiRenderer *renderer, const char *shader_path);
void zui_renderer_shutdown(ZuiRenderer *renderer);

void zui_renderer_begin(ZuiRenderer *renderer, int width, int height);
void zui_renderer_end(ZuiRenderer *renderer);
void zui_renderer_clear(ZuiRenderer *renderer, ZuiColor color);

void zui_renderer_draw_rect(ZuiRenderer *renderer, ZuiRect rect, ZuiColor color);
void zui_renderer_draw_rounded_rect(ZuiRenderer *renderer, ZuiRect rect,
                                     ZuiColor color, float radius);
void zui_renderer_draw_rounded_rect_top(ZuiRenderer *renderer, ZuiRect rect,
                                         ZuiColor color, float radius);
void zui_renderer_draw_rounded_rect_bottom(ZuiRenderer *renderer, ZuiRect rect,
                                            ZuiColor color, float radius);
void zui_renderer_draw_rounded_rect_outline(ZuiRenderer *renderer, ZuiRect rect,
                                             ZuiColor color, float radius,
                                             float thickness);

ZuiTexture zui_texture_create(const uint8_t *data, int width, int height);
void zui_texture_destroy(ZuiTexture *texture);
void zui_renderer_draw_texture(ZuiRenderer *renderer, ZuiTexture *texture,
                                ZuiRect rect, ZuiColor tint);
void zui_renderer_draw_glyph(ZuiRenderer *renderer, ZuiTexture *atlas,
                              ZuiRect rect, float u0, float v0, float u1, float v1,
                              ZuiColor color);

void zui_renderer_push_clip(ZuiRenderer *renderer, ZuiRect rect, float radius);
void zui_renderer_pop_clip(ZuiRenderer *renderer);

void zui_renderer_draw_window(ZuiRenderer *renderer, ZuiRect rect,
                               const ZuiWindowStyle *style);

#define ZUI_RECT(x, y, w, h) ((ZuiRect){(x), (y), (w), (h)})

#endif
