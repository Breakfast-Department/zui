#ifndef ZUI_RENDERER_H
#define ZUI_RENDERER_H

#include <glad/glad.h>
#include <stdbool.h>
#include <stdint.h>
#include <zui/color.h>
#include <zui/path.h>

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

typedef struct ZuiRenderer {
  GLuint rect_shader;
  GLuint rect_vao;
  GLuint rect_vbo;

  GLuint tex_shader;
  GLuint tex_vao;
  GLuint tex_vbo;

  GLuint glyph_shader;

  GLuint circle_shader;
  GLuint circle_vao;
  GLuint circle_vbo;

  GLuint arc_shader;
  GLuint arc_vao;
  GLuint arc_vbo;

  GLuint poly_shader;
  GLuint poly_vao;
  GLuint poly_vbo;
  int poly_vbo_capacity;

  int viewport_width;
  int viewport_height;

  ZuiClipState clip_stack[ZUI_MAX_CLIP_STACK];
  int clip_stack_top;

  int stencil_level;
} ZuiRenderer;

bool zui_renderer_init(ZuiRenderer *renderer);
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

void zui_renderer_draw_circle(ZuiRenderer *renderer, float cx, float cy,
                               float radius, ZuiColor color);
void zui_renderer_draw_circle_outline(ZuiRenderer *renderer, float cx, float cy,
                                       float radius, float thickness, ZuiColor color);
void zui_renderer_draw_arc(ZuiRenderer *renderer, float cx, float cy,
                            float radius, float start_angle, float end_angle,
                            ZuiColor color);
void zui_renderer_draw_arc_outline(ZuiRenderer *renderer, float cx, float cy,
                                    float radius, float start_angle, float end_angle,
                                    float thickness, ZuiColor color);

ZuiTexture zui_texture_create(const uint8_t *data, int width, int height);
ZuiTexture zui_texture_create_empty(int width, int height);
ZuiTexture zui_texture_load(const char *path);
void zui_texture_destroy(ZuiTexture *texture);
void zui_texture_update(ZuiTexture *texture, const uint8_t *data,
                        int x, int y, int width, int height);
void zui_renderer_draw_texture(ZuiRenderer *renderer, ZuiTexture *texture,
                                ZuiRect rect, ZuiColor tint);
void zui_renderer_draw_glyph(ZuiRenderer *renderer, ZuiTexture *atlas,
                              ZuiRect rect, float u0, float v0, float u1, float v1,
                              ZuiColor color);

void zui_renderer_push_clip(ZuiRenderer *renderer, ZuiRect rect, float radius);
void zui_renderer_pop_clip(ZuiRenderer *renderer);

void zui_renderer_push_path_clip(ZuiRenderer *renderer, ZuiPath *path);
void zui_renderer_pop_path_clip(ZuiRenderer *renderer);

void zui_renderer_draw_path(ZuiRenderer *renderer, ZuiPath *path, ZuiColor color);
void zui_renderer_draw_path_stroke(ZuiRenderer *renderer, ZuiPath *path,
                                    ZuiColor color, float thickness);

#define ZUI_RECT(x, y, w, h) ((ZuiRect){(x), (y), (w), (h)})

#endif
