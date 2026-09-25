#include <zui/internal/renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern const unsigned char *zui_internal_resource_get(const char *path, size_t *size);

static GLuint compile_shader(GLenum type, const char *source)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[512];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    fprintf(stderr, "ZUI: Shader compile error: %s\n", log);
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

static char *get_shader_source(const char *name)
{
  char res_path[256];
  snprintf(res_path, sizeof(res_path), "res:/zui/shaders/%s", name);

  size_t size;
  const unsigned char *data = zui_internal_resource_get(res_path, &size);
  if (data && size > 0) {
    char *src = malloc(size + 1);
    if (src) {
      memcpy(src, data, size);
      src[size] = '\0';
      return src;
    }
  }

  fprintf(stderr, "ZUI: Shader not found: %s\n", name);
  return NULL;
}

static GLuint load_shader_program(const char *vert_name, const char *frag_name)
{
  char *vert_src = get_shader_source(vert_name);
  if (!vert_src) return 0;

  char *frag_src = get_shader_source(frag_name);
  if (!frag_src) {
    free(vert_src);
    return 0;
  }

  GLuint vert = compile_shader(GL_VERTEX_SHADER, vert_src);
  free(vert_src);
  if (!vert) {
    free(frag_src);
    return 0;
  }

  GLuint frag = compile_shader(GL_FRAGMENT_SHADER, frag_src);
  free(frag_src);
  if (!frag) {
    glDeleteShader(vert);
    return 0;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char log[512];
    glGetProgramInfoLog(program, sizeof(log), NULL, log);
    fprintf(stderr, "ZUI: Shader link error: %s\n", log);
    glDeleteProgram(program);
    program = 0;
  }

  glDeleteShader(vert);
  glDeleteShader(frag);
  return program;
}

bool zui_renderer_init(ZuiRenderer *renderer)
{
  memset(renderer, 0, sizeof(*renderer));
  renderer->clip_stack_top = -1;

  if (!gladLoadGL()) {
    fprintf(stderr, "ZUI: Failed to load OpenGL\n");
    return false;
  }

  renderer->rect_shader = load_shader_program("rect.vert", "rect.frag");
  if (!renderer->rect_shader) {
    return false;
  }

  renderer->tex_shader = load_shader_program("texture.vert", "texture.frag");
  if (!renderer->tex_shader) {
    glDeleteProgram(renderer->rect_shader);
    return false;
  }

  renderer->glyph_shader = load_shader_program("glyph.vert", "glyph.frag");
  if (!renderer->glyph_shader) {
    glDeleteProgram(renderer->tex_shader);
    glDeleteProgram(renderer->rect_shader);
    return false;
  }

  renderer->circle_shader = load_shader_program("circle.vert", "circle.frag");
  if (!renderer->circle_shader) {
    glDeleteProgram(renderer->glyph_shader);
    glDeleteProgram(renderer->tex_shader);
    glDeleteProgram(renderer->rect_shader);
    return false;
  }

  renderer->arc_shader = load_shader_program("arc.vert", "arc.frag");
  if (!renderer->arc_shader) {
    glDeleteProgram(renderer->circle_shader);
    glDeleteProgram(renderer->glyph_shader);
    glDeleteProgram(renderer->tex_shader);
    glDeleteProgram(renderer->rect_shader);
    return false;
  }

  static const float quad_verts[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
  };

  glGenVertexArrays(1, &renderer->rect_vao);
  glGenBuffers(1, &renderer->rect_vbo);

  glBindVertexArray(renderer->rect_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->rect_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  static const float tex_verts[] = {
    0.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 0.0f,  1.0f, 0.0f,
    1.0f, 1.0f,  1.0f, 1.0f,
    0.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 1.0f,  1.0f, 1.0f,
    0.0f, 1.0f,  0.0f, 1.0f,
  };

  glGenVertexArrays(1, &renderer->tex_vao);
  glGenBuffers(1, &renderer->tex_vbo);

  glBindVertexArray(renderer->tex_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->tex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tex_verts), tex_verts, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void*)(2 * sizeof(float)));

  glBindVertexArray(0);

  glGenVertexArrays(1, &renderer->circle_vao);
  glGenBuffers(1, &renderer->circle_vbo);

  glBindVertexArray(renderer->circle_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->circle_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  glGenVertexArrays(1, &renderer->arc_vao);
  glGenBuffers(1, &renderer->arc_vbo);

  glBindVertexArray(renderer->arc_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->arc_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  renderer->poly_shader = load_shader_program("poly.vert", "poly.frag");
  if (!renderer->poly_shader) {
    glDeleteProgram(renderer->arc_shader);
    glDeleteProgram(renderer->circle_shader);
    glDeleteProgram(renderer->glyph_shader);
    glDeleteProgram(renderer->tex_shader);
    glDeleteProgram(renderer->rect_shader);
    return false;
  }

  glGenVertexArrays(1, &renderer->poly_vao);
  glGenBuffers(1, &renderer->poly_vbo);
  renderer->poly_vbo_capacity = 0;

  glBindVertexArray(renderer->poly_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->poly_vbo);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  return true;
}

void zui_renderer_shutdown(ZuiRenderer *renderer)
{
  if (renderer->poly_vbo) glDeleteBuffers(1, &renderer->poly_vbo);
  if (renderer->poly_vao) glDeleteVertexArrays(1, &renderer->poly_vao);
  if (renderer->poly_shader) glDeleteProgram(renderer->poly_shader);
  if (renderer->arc_vbo) glDeleteBuffers(1, &renderer->arc_vbo);
  if (renderer->arc_vao) glDeleteVertexArrays(1, &renderer->arc_vao);
  if (renderer->arc_shader) glDeleteProgram(renderer->arc_shader);
  if (renderer->circle_vbo) glDeleteBuffers(1, &renderer->circle_vbo);
  if (renderer->circle_vao) glDeleteVertexArrays(1, &renderer->circle_vao);
  if (renderer->circle_shader) glDeleteProgram(renderer->circle_shader);
  if (renderer->glyph_shader) glDeleteProgram(renderer->glyph_shader);
  if (renderer->tex_vbo) glDeleteBuffers(1, &renderer->tex_vbo);
  if (renderer->tex_vao) glDeleteVertexArrays(1, &renderer->tex_vao);
  if (renderer->tex_shader) glDeleteProgram(renderer->tex_shader);
  if (renderer->rect_vbo) glDeleteBuffers(1, &renderer->rect_vbo);
  if (renderer->rect_vao) glDeleteVertexArrays(1, &renderer->rect_vao);
  if (renderer->rect_shader) glDeleteProgram(renderer->rect_shader);
}

void zui_renderer_begin(ZuiRenderer *renderer, int width, int height)
{
  renderer->viewport_width = width;
  renderer->viewport_height = height;
  renderer->clip_stack_top = -1;
  renderer->stencil_level = 0;

  glViewport(0, 0, width, height);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_STENCIL_TEST);
}

void zui_renderer_end(ZuiRenderer *renderer)
{
  (void)renderer;
  glDisable(GL_BLEND);
}

void zui_renderer_clear(ZuiRenderer *renderer, ZuiColor color)
{
  (void)renderer;
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

static void apply_clip_uniforms(ZuiRenderer *renderer, GLuint shader)
{
  bool clip_enabled = renderer->clip_stack_top >= 0 &&
                      renderer->clip_stack[renderer->clip_stack_top].radius > 0.0f;

  GLint enabled_loc = glGetUniformLocation(shader, "u_clip_enabled");
  glUniform1i(enabled_loc, clip_enabled ? 1 : 0);

  if (clip_enabled) {
    ZuiClipState *clip = &renderer->clip_stack[renderer->clip_stack_top];
    GLint rect_loc = glGetUniformLocation(shader, "u_clip_rect");
    glUniform4f(rect_loc, clip->rect.x, clip->rect.y,
                clip->rect.w, clip->rect.h);
    GLint radius_loc = glGetUniformLocation(shader, "u_clip_radius");
    glUniform1f(radius_loc, clip->radius);
  }
}


static void draw_rect_internal(ZuiRenderer *renderer, ZuiRect rect,
                                ZuiColor color, float tl, float tr,
                                float br, float bl, float outline)
{
  glUseProgram(renderer->rect_shader);
  glBindVertexArray(renderer->rect_vao);

  GLint res_loc = glGetUniformLocation(renderer->rect_shader, "u_resolution");
  glUniform2f(res_loc, (float)renderer->viewport_width,
              (float)renderer->viewport_height);

  apply_clip_uniforms(renderer, renderer->rect_shader);

  glVertexAttrib2f(1, rect.x, rect.y);
  glVertexAttrib2f(2, rect.w, rect.h);
  glVertexAttrib4f(3, color.r, color.g, color.b, color.a);
  glVertexAttrib4f(4, tl, tr, br, bl);
  glVertexAttrib1f(5, outline);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray(0);
  glUseProgram(0);
}

void zui_renderer_draw_rect(ZuiRenderer *renderer, ZuiRect rect, ZuiColor color)
{
  draw_rect_internal(renderer, rect, color, 0, 0, 0, 0, 0);
}

void zui_renderer_draw_rounded_rect(ZuiRenderer *renderer, ZuiRect rect,
                                     ZuiColor color, float radius)
{
  draw_rect_internal(renderer, rect, color, radius, radius, radius, radius, 0);
}

void zui_renderer_draw_rounded_rect_top(ZuiRenderer *renderer, ZuiRect rect,
                                         ZuiColor color, float radius)
{
  draw_rect_internal(renderer, rect, color, radius, radius, 0, 0, 0);
}

void zui_renderer_draw_rounded_rect_bottom(ZuiRenderer *renderer, ZuiRect rect,
                                            ZuiColor color, float radius)
{
  draw_rect_internal(renderer, rect, color, 0, 0, radius, radius, 0);
}

void zui_renderer_draw_rounded_rect_outline(ZuiRenderer *renderer, ZuiRect rect,
                                             ZuiColor color, float radius,
                                             float thickness)
{
  draw_rect_internal(renderer, rect, color, radius, radius, radius, radius, thickness);
}

static void draw_circle_internal(ZuiRenderer *renderer, float cx, float cy,
                                  float radius, ZuiColor color, float thickness)
{
  glUseProgram(renderer->circle_shader);
  glBindVertexArray(renderer->circle_vao);

  GLint res_loc = glGetUniformLocation(renderer->circle_shader, "u_resolution");
  glUniform2f(res_loc, (float)renderer->viewport_width,
              (float)renderer->viewport_height);

  apply_clip_uniforms(renderer, renderer->circle_shader);

  glVertexAttrib2f(1, cx, cy);
  glVertexAttrib1f(2, radius);
  glVertexAttrib4f(3, color.r, color.g, color.b, color.a);
  glVertexAttrib1f(4, thickness);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray(0);
  glUseProgram(0);
}

void zui_renderer_draw_circle(ZuiRenderer *renderer, float cx, float cy,
                               float radius, ZuiColor color)
{
  draw_circle_internal(renderer, cx, cy, radius, color, 0);
}

void zui_renderer_draw_circle_outline(ZuiRenderer *renderer, float cx, float cy,
                                       float radius, float thickness, ZuiColor color)
{
  draw_circle_internal(renderer, cx, cy, radius, color, thickness);
}

static void draw_arc_internal(ZuiRenderer *renderer, float cx, float cy,
                               float radius, float start_angle, float end_angle,
                               ZuiColor color, float thickness)
{
  glUseProgram(renderer->arc_shader);
  glBindVertexArray(renderer->arc_vao);

  GLint res_loc = glGetUniformLocation(renderer->arc_shader, "u_resolution");
  glUniform2f(res_loc, (float)renderer->viewport_width,
              (float)renderer->viewport_height);

  apply_clip_uniforms(renderer, renderer->arc_shader);

  glVertexAttrib2f(1, cx, cy);
  glVertexAttrib1f(2, radius);
  glVertexAttrib4f(3, color.r, color.g, color.b, color.a);
  glVertexAttrib1f(4, start_angle);
  glVertexAttrib1f(5, end_angle);
  glVertexAttrib1f(6, thickness);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray(0);
  glUseProgram(0);
}

void zui_renderer_draw_arc(ZuiRenderer *renderer, float cx, float cy,
                            float radius, float start_angle, float end_angle,
                            ZuiColor color)
{
  draw_arc_internal(renderer, cx, cy, radius, start_angle, end_angle, color, 0);
}

void zui_renderer_draw_arc_outline(ZuiRenderer *renderer, float cx, float cy,
                                    float radius, float start_angle, float end_angle,
                                    float thickness, ZuiColor color)
{
  draw_arc_internal(renderer, cx, cy, radius, start_angle, end_angle, color, thickness);
}

ZuiTexture zui_texture_create(const uint8_t *data, int width, int height)
{
  ZuiTexture texture = {0, width, height};

  glGenTextures(1, &texture.id);
  glBindTexture(GL_TEXTURE_2D, texture.id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, data);

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}

extern unsigned char *stbi_load(const char *filename, int *x, int *y, int *comp, int req_comp);
extern void stbi_image_free(void *data);

ZuiTexture zui_texture_load(const char *path)
{
  if (!path) return (ZuiTexture){0, 0, 0};

  int width, height, channels;
  unsigned char *data = stbi_load(path, &width, &height, &channels, 4);
  if (!data) return (ZuiTexture){0, 0, 0};

  ZuiTexture texture = zui_texture_create(data, width, height);
  stbi_image_free(data);

  return texture;
}

void zui_texture_destroy(ZuiTexture *texture)
{
  if (texture && texture->id) {
    glDeleteTextures(1, &texture->id);
    texture->id = 0;
  }
}

ZuiTexture zui_texture_create_empty(int width, int height)
{
  ZuiTexture texture = {0, width, height};

  glGenTextures(1, &texture.id);
  glBindTexture(GL_TEXTURE_2D, texture.id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}

void zui_texture_update(ZuiTexture *texture, const uint8_t *data,
                        int x, int y, int width, int height)
{
  if (!texture || !texture->id || !data) return;

  glBindTexture(GL_TEXTURE_2D, texture->id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height,
                  GL_RGBA, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void zui_renderer_draw_texture(ZuiRenderer *renderer, ZuiTexture *texture,
                                ZuiRect rect, ZuiColor tint)
{
  if (!texture || !texture->id) return;

  glUseProgram(renderer->tex_shader);
  glBindVertexArray(renderer->tex_vao);

  GLint res_loc = glGetUniformLocation(renderer->tex_shader, "u_resolution");
  glUniform2f(res_loc, (float)renderer->viewport_width,
              (float)renderer->viewport_height);

  GLint rect_loc = glGetUniformLocation(renderer->tex_shader, "u_rect");
  glUniform4f(rect_loc, rect.x, rect.y, rect.w, rect.h);

  GLint tint_loc = glGetUniformLocation(renderer->tex_shader, "u_tint");
  glUniform4f(tint_loc, tint.r, tint.g, tint.b, tint.a);

  apply_clip_uniforms(renderer, renderer->tex_shader);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture->id);
  GLint tex_loc = glGetUniformLocation(renderer->tex_shader, "u_texture");
  glUniform1i(tex_loc, 0);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void zui_renderer_draw_glyph(ZuiRenderer *renderer, ZuiTexture *atlas,
                              ZuiRect rect, float u0, float v0, float u1, float v1,
                              ZuiColor color)
{
  if (!atlas || !atlas->id) return;

  glUseProgram(renderer->glyph_shader);
  glBindVertexArray(renderer->tex_vao);

  GLint res_loc = glGetUniformLocation(renderer->glyph_shader, "u_resolution");
  glUniform2f(res_loc, (float)renderer->viewport_width,
              (float)renderer->viewport_height);

  GLint rect_loc = glGetUniformLocation(renderer->glyph_shader, "u_rect");
  glUniform4f(rect_loc, rect.x, rect.y, rect.w, rect.h);

  GLint uv_loc = glGetUniformLocation(renderer->glyph_shader, "u_uv_rect");
  glUniform4f(uv_loc, u0, v0, u1 - u0, v1 - v0);

  GLint color_loc = glGetUniformLocation(renderer->glyph_shader, "u_color");
  glUniform4f(color_loc, color.r, color.g, color.b, color.a);

  apply_clip_uniforms(renderer, renderer->glyph_shader);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, atlas->id);
  GLint tex_loc = glGetUniformLocation(renderer->glyph_shader, "u_texture");
  glUniform1i(tex_loc, 0);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

static void intersect_rect_with_edges(ZuiRect a, ZuiRect b, ZuiRect *result,
                                       bool *left_clipped, bool *right_clipped,
                                       bool *top_clipped, bool *bottom_clipped)
{
  float x1_a = a.x, x1_b = b.x;
  float y1_a = a.y, y1_b = b.y;
  float x2_a = a.x + a.w, x2_b = b.x + b.w;
  float y2_a = a.y + a.h, y2_b = b.y + b.h;

  *left_clipped = x1_a < x1_b;
  *top_clipped = y1_a < y1_b;
  *right_clipped = x2_a > x2_b;
  *bottom_clipped = y2_a > y2_b;

  float x1 = x1_a > x1_b ? x1_a : x1_b;
  float y1 = y1_a > y1_b ? y1_a : y1_b;
  float x2 = x2_a < x2_b ? x2_a : x2_b;
  float y2 = y2_a < y2_b ? y2_a : y2_b;

  result->x = x1;
  result->y = y1;
  result->w = x2 > x1 ? x2 - x1 : 0;
  result->h = y2 > y1 ? y2 - y1 : 0;
}

void zui_renderer_push_clip(ZuiRenderer *renderer, ZuiRect rect, float radius)
{
  if (renderer->clip_stack_top >= ZUI_MAX_CLIP_STACK - 1) return;

  ZuiRect viewport_rect = {0, 0, (float)renderer->viewport_width, (float)renderer->viewport_height};
  ZuiRect clipped_rect;
  bool left_clip = false, right_clip = false, top_clip = false, bottom_clip = false;
  bool l, r, t, b;

  intersect_rect_with_edges(rect, viewport_rect, &clipped_rect, &l, &r, &t, &b);
  left_clip |= l; right_clip |= r; top_clip |= t; bottom_clip |= b;

  if (renderer->clip_stack_top >= 0) {
    ZuiClipState *parent = &renderer->clip_stack[renderer->clip_stack_top];
    ZuiRect temp = clipped_rect;
    intersect_rect_with_edges(temp, parent->rect, &clipped_rect, &l, &r, &t, &b);
    left_clip |= l; right_clip |= r; top_clip |= t; bottom_clip |= b;
  }

  bool was_clipped = left_clip || right_clip || top_clip || bottom_clip;

  ZuiClipState new_clip;
  new_clip.rect = clipped_rect;
  new_clip.radius = was_clipped ? 0 : radius;

  renderer->clip_stack_top++;
  renderer->clip_stack[renderer->clip_stack_top] = new_clip;

  glEnable(GL_SCISSOR_TEST);
  float sx = clipped_rect.x + (left_clip ? 1.0f : 0);
  float sy = clipped_rect.y + (top_clip ? 1.0f : 0);
  float ex = clipped_rect.x + clipped_rect.w - (right_clip ? 1.0f : 0);
  float ey = clipped_rect.y + clipped_rect.h - (bottom_clip ? 1.0f : 0);
  int isx = (int)sx;
  int isy = (int)sy;
  int sw = (int)ex - isx;
  int sh = (int)ey - isy;
  if (sw < 0) sw = 0;
  if (sh < 0) sh = 0;
  int scissor_y = renderer->viewport_height - isy - sh;
  glScissor(isx, scissor_y, sw, sh);
}

void zui_renderer_pop_clip(ZuiRenderer *renderer)
{
  if (renderer->clip_stack_top >= 0) {
    renderer->clip_stack_top--;
  }

  if (renderer->clip_stack_top >= 0) {
    ZuiClipState *clip = &renderer->clip_stack[renderer->clip_stack_top];
    int scissor_y = renderer->viewport_height - (int)(clip->rect.y + clip->rect.h);
    glScissor((int)clip->rect.x, scissor_y, (int)clip->rect.w, (int)clip->rect.h);
  } else {
    glDisable(GL_SCISSOR_TEST);
  }
}

static void draw_path_to_stencil(ZuiRenderer *renderer, ZuiPath *path);

void zui_renderer_push_path_clip(ZuiRenderer *renderer, ZuiPath *path)
{
  if (!renderer || !path) return;

  if (renderer->stencil_level == 0) {
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
  }

  renderer->stencil_level++;

  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glStencilFunc(GL_ALWAYS, renderer->stencil_level, 0xFF);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

  draw_path_to_stencil(renderer, path);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glStencilFunc(GL_EQUAL, renderer->stencil_level, 0xFF);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void zui_renderer_pop_path_clip(ZuiRenderer *renderer)
{
  if (!renderer || renderer->stencil_level <= 0) return;

  renderer->stencil_level--;

  if (renderer->stencil_level == 0) {
    glDisable(GL_STENCIL_TEST);
  } else {
    glStencilFunc(GL_EQUAL, renderer->stencil_level, 0xFF);
  }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PATH_CURVE_SEGMENTS 32

typedef struct {
  float *data;
  int count;
  int capacity;
} VertexBuffer;

static void vb_init(VertexBuffer *vb)
{
  vb->data = NULL;
  vb->count = 0;
  vb->capacity = 0;
}

static void vb_free(VertexBuffer *vb)
{
  free(vb->data);
  vb->data = NULL;
  vb->count = 0;
  vb->capacity = 0;
}

static void vb_push(VertexBuffer *vb, float x, float y)
{
  if (vb->count + 2 > vb->capacity) {
    int new_cap = vb->capacity == 0 ? 64 : vb->capacity * 2;
    float *new_data = realloc(vb->data, (size_t)new_cap * sizeof(float));
    if (!new_data) return;
    vb->data = new_data;
    vb->capacity = new_cap;
  }
  vb->data[vb->count++] = x;
  vb->data[vb->count++] = y;
}

static void tessellate_quad_bezier(VertexBuffer *vb, float x0, float y0,
                                    float cx, float cy, float x1, float y1)
{
  for (int i = 1; i <= PATH_CURVE_SEGMENTS; i++) {
    float t = (float)i / PATH_CURVE_SEGMENTS;
    float mt = 1.0f - t;
    float x = mt * mt * x0 + 2.0f * mt * t * cx + t * t * x1;
    float y = mt * mt * y0 + 2.0f * mt * t * cy + t * t * y1;
    vb_push(vb, x, y);
  }
}

static void tessellate_cubic_bezier(VertexBuffer *vb, float x0, float y0,
                                     float cx1, float cy1, float cx2, float cy2,
                                     float x1, float y1)
{
  for (int i = 1; i <= PATH_CURVE_SEGMENTS; i++) {
    float t = (float)i / PATH_CURVE_SEGMENTS;
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;
    float t2 = t * t;
    float t3 = t2 * t;
    float x = mt3 * x0 + 3.0f * mt2 * t * cx1 + 3.0f * mt * t2 * cx2 + t3 * x1;
    float y = mt3 * y0 + 3.0f * mt2 * t * cy1 + 3.0f * mt * t2 * cy2 + t3 * y1;
    vb_push(vb, x, y);
  }
}

static void tessellate_arc(VertexBuffer *vb, float cx, float cy, float radius,
                            float start_angle, float end_angle)
{
  float delta = end_angle - start_angle;
  int segments = (int)(fabsf(delta) / (float)M_PI * PATH_CURVE_SEGMENTS);
  if (segments < 4) segments = 4;

  for (int i = 0; i <= segments; i++) {
    float t = (float)i / (float)segments;
    float angle = start_angle + t * delta;
    float x = cx + radius * cosf(angle);
    float y = cy + radius * sinf(angle);
    vb_push(vb, x, y);
  }
}

static void path_to_vertices(ZuiPath *path, VertexBuffer *vb)
{
  int cmd_count;
  const ZuiPathCommand *cmds = zui_path_get_commands(path, &cmd_count);
  if (!cmds || cmd_count == 0) return;

  float curr_x = 0, curr_y = 0;

  for (int i = 0; i < cmd_count; i++) {
    const ZuiPathCommand *cmd = &cmds[i];

    switch (cmd->type) {
      case ZUI_PATH_MOVE_TO:
        vb_push(vb, cmd->x, cmd->y);
        curr_x = cmd->x;
        curr_y = cmd->y;
        break;

      case ZUI_PATH_LINE_TO:
        vb_push(vb, cmd->x, cmd->y);
        curr_x = cmd->x;
        curr_y = cmd->y;
        break;

      case ZUI_PATH_QUAD_TO:
        tessellate_quad_bezier(vb, curr_x, curr_y,
                               cmd->cx1, cmd->cy1, cmd->x, cmd->y);
        curr_x = cmd->x;
        curr_y = cmd->y;
        break;

      case ZUI_PATH_CUBIC_TO:
        tessellate_cubic_bezier(vb, curr_x, curr_y,
                                cmd->cx1, cmd->cy1,
                                cmd->cx2, cmd->cy2,
                                cmd->x, cmd->y);
        curr_x = cmd->x;
        curr_y = cmd->y;
        break;

      case ZUI_PATH_ARC_TO:
        tessellate_arc(vb, cmd->x, cmd->y, cmd->radius, cmd->cx1, cmd->cy1);
        curr_x = cmd->x + cmd->radius * cosf(cmd->cy1);
        curr_y = cmd->y + cmd->radius * sinf(cmd->cy1);
        break;

      case ZUI_PATH_CLOSE:
        break;
    }
  }
}

static float cross2d(float ax, float ay, float bx, float by)
{
  return ax * by - ay * bx;
}

static bool is_convex_polygon(const float *verts, int vert_count)
{
  if (vert_count < 3) return false;

  int sign = 0;
  for (int i = 0; i < vert_count; i++) {
    int j = (i + 1) % vert_count;
    int k = (i + 2) % vert_count;

    float ax = verts[j * 2] - verts[i * 2];
    float ay = verts[j * 2 + 1] - verts[i * 2 + 1];
    float bx = verts[k * 2] - verts[j * 2];
    float by = verts[k * 2 + 1] - verts[j * 2 + 1];

    float c = cross2d(ax, ay, bx, by);
    int s = c > 0 ? 1 : (c < 0 ? -1 : 0);

    if (s != 0) {
      if (sign == 0) {
        sign = s;
      } else if (sign != s) {
        return false;
      }
    }
  }
  return true;
}

static void triangulate_fan(const float *verts, int vert_count, VertexBuffer *out)
{
  for (int i = 1; i < vert_count - 1; i++) {
    vb_push(out, verts[0], verts[1]);
    vb_push(out, verts[i * 2], verts[i * 2 + 1]);
    vb_push(out, verts[(i + 1) * 2], verts[(i + 1) * 2 + 1]);
  }
}

static bool point_in_triangle(float px, float py,
                               float ax, float ay, float bx, float by, float cx, float cy)
{
  float v0x = cx - ax, v0y = cy - ay;
  float v1x = bx - ax, v1y = by - ay;
  float v2x = px - ax, v2y = py - ay;

  float dot00 = v0x * v0x + v0y * v0y;
  float dot01 = v0x * v1x + v0y * v1y;
  float dot02 = v0x * v2x + v0y * v2y;
  float dot11 = v1x * v1x + v1y * v1y;
  float dot12 = v1x * v2x + v1y * v2y;

  float inv_denom = 1.0f / (dot00 * dot11 - dot01 * dot01);
  float u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
  float v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

  return (u >= 0) && (v >= 0) && (u + v <= 1);
}

static void triangulate_ear_clipping(const float *verts, int vert_count, VertexBuffer *out)
{
  if (vert_count < 3) return;

  int *indices = malloc((size_t)vert_count * sizeof(int));
  if (!indices) return;

  for (int i = 0; i < vert_count; i++) indices[i] = i;

  int n = vert_count;
  while (n > 3) {
    bool found_ear = false;

    for (int i = 0; i < n; i++) {
      int prev = (i + n - 1) % n;
      int next = (i + 1) % n;

      int ip = indices[prev];
      int ic = indices[i];
      int in = indices[next];

      float ax = verts[ip * 2], ay = verts[ip * 2 + 1];
      float bx = verts[ic * 2], by = verts[ic * 2 + 1];
      float cx = verts[in * 2], cy = verts[in * 2 + 1];

      float c = cross2d(bx - ax, by - ay, cx - bx, cy - by);
      if (c >= 0) continue;

      bool ear = true;
      for (int j = 0; j < n; j++) {
        if (j == prev || j == i || j == next) continue;
        int ij = indices[j];
        if (point_in_triangle(verts[ij * 2], verts[ij * 2 + 1],
                              ax, ay, bx, by, cx, cy)) {
          ear = false;
          break;
        }
      }

      if (ear) {
        vb_push(out, ax, ay);
        vb_push(out, bx, by);
        vb_push(out, cx, cy);

        for (int j = i; j < n - 1; j++) {
          indices[j] = indices[j + 1];
        }
        n--;
        found_ear = true;
        break;
      }
    }

    if (!found_ear) break;
  }

  if (n == 3) {
    vb_push(out, verts[indices[0] * 2], verts[indices[0] * 2 + 1]);
    vb_push(out, verts[indices[1] * 2], verts[indices[1] * 2 + 1]);
    vb_push(out, verts[indices[2] * 2], verts[indices[2] * 2 + 1]);
  }

  free(indices);
}

static void draw_path_to_stencil(ZuiRenderer *renderer, ZuiPath *path)
{
  if (!path) return;

  VertexBuffer outline;
  vb_init(&outline);
  path_to_vertices(path, &outline);

  int vert_count = outline.count / 2;
  if (vert_count < 3) {
    vb_free(&outline);
    return;
  }

  VertexBuffer triangles;
  vb_init(&triangles);

  if (is_convex_polygon(outline.data, vert_count)) {
    triangulate_fan(outline.data, vert_count, &triangles);
  } else {
    triangulate_ear_clipping(outline.data, vert_count, &triangles);
  }

  vb_free(&outline);

  if (triangles.count == 0) {
    vb_free(&triangles);
    return;
  }

  glUseProgram(renderer->poly_shader);
  glBindVertexArray(renderer->poly_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->poly_vbo);

  int needed = triangles.count * (int)sizeof(float);
  if (needed > renderer->poly_vbo_capacity) {
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)needed, triangles.data, GL_DYNAMIC_DRAW);
    renderer->poly_vbo_capacity = needed;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)needed, triangles.data);
  }

  GLint res_loc = glGetUniformLocation(renderer->poly_shader, "u_resolution");
  glUniform2f(res_loc, (float)renderer->viewport_width,
              (float)renderer->viewport_height);

  GLint color_loc = glGetUniformLocation(renderer->poly_shader, "u_color");
  glUniform4f(color_loc, 1.0f, 1.0f, 1.0f, 1.0f);

  glDrawArrays(GL_TRIANGLES, 0, triangles.count / 2);

  glBindVertexArray(0);
  glUseProgram(0);

  vb_free(&triangles);
}

void zui_renderer_draw_path(ZuiRenderer *renderer, ZuiPath *path, ZuiColor color)
{
  if (!path) return;

  VertexBuffer outline;
  vb_init(&outline);
  path_to_vertices(path, &outline);

  int vert_count = outline.count / 2;
  if (vert_count < 3) {
    vb_free(&outline);
    return;
  }

  VertexBuffer triangles;
  vb_init(&triangles);

  if (is_convex_polygon(outline.data, vert_count)) {
    triangulate_fan(outline.data, vert_count, &triangles);
  } else {
    triangulate_ear_clipping(outline.data, vert_count, &triangles);
  }

  vb_free(&outline);

  if (triangles.count == 0) {
    vb_free(&triangles);
    return;
  }

  glUseProgram(renderer->poly_shader);
  glBindVertexArray(renderer->poly_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->poly_vbo);

  int needed = triangles.count * (int)sizeof(float);
  if (needed > renderer->poly_vbo_capacity) {
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)needed, triangles.data, GL_DYNAMIC_DRAW);
    renderer->poly_vbo_capacity = needed;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)needed, triangles.data);
  }

  GLint res_loc = glGetUniformLocation(renderer->poly_shader, "u_resolution");
  glUniform2f(res_loc, (float)renderer->viewport_width,
              (float)renderer->viewport_height);

  GLint color_loc = glGetUniformLocation(renderer->poly_shader, "u_color");
  glUniform4f(color_loc, color.r * color.a, color.g * color.a,
              color.b * color.a, color.a);

  apply_clip_uniforms(renderer, renderer->poly_shader);

  glDrawArrays(GL_TRIANGLES, 0, triangles.count / 2);

  glBindVertexArray(0);
  glUseProgram(0);

  vb_free(&triangles);
}

void zui_renderer_draw_path_stroke(ZuiRenderer *renderer, ZuiPath *path,
                                    ZuiColor color, float thickness)
{
  if (!path || thickness <= 0) return;

  VertexBuffer outline;
  vb_init(&outline);
  path_to_vertices(path, &outline);

  int vert_count = outline.count / 2;
  if (vert_count < 2) {
    vb_free(&outline);
    return;
  }

  VertexBuffer triangles;
  vb_init(&triangles);

  float half = thickness / 2.0f;

  for (int i = 0; i < vert_count; i++) {
    int j = (i + 1) % vert_count;

    float x0 = outline.data[i * 2];
    float y0 = outline.data[i * 2 + 1];
    float x1 = outline.data[j * 2];
    float y1 = outline.data[j * 2 + 1];

    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.0001f) continue;

    float nx = -dy / len * half;
    float ny = dx / len * half;

    vb_push(&triangles, x0 + nx, y0 + ny);
    vb_push(&triangles, x0 - nx, y0 - ny);
    vb_push(&triangles, x1 + nx, y1 + ny);

    vb_push(&triangles, x0 - nx, y0 - ny);
    vb_push(&triangles, x1 - nx, y1 - ny);
    vb_push(&triangles, x1 + nx, y1 + ny);
  }

  vb_free(&outline);

  if (triangles.count == 0) {
    vb_free(&triangles);
    return;
  }

  glUseProgram(renderer->poly_shader);
  glBindVertexArray(renderer->poly_vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->poly_vbo);

  int needed = triangles.count * (int)sizeof(float);
  if (needed > renderer->poly_vbo_capacity) {
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)needed, triangles.data, GL_DYNAMIC_DRAW);
    renderer->poly_vbo_capacity = needed;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)needed, triangles.data);
  }

  GLint res_loc = glGetUniformLocation(renderer->poly_shader, "u_resolution");
  glUniform2f(res_loc, (float)renderer->viewport_width,
              (float)renderer->viewport_height);

  GLint color_loc = glGetUniformLocation(renderer->poly_shader, "u_color");
  glUniform4f(color_loc, color.r * color.a, color.g * color.a,
              color.b * color.a, color.a);

  apply_clip_uniforms(renderer, renderer->poly_shader);

  glDrawArrays(GL_TRIANGLES, 0, triangles.count / 2);

  glBindVertexArray(0);
  glUseProgram(0);

  vb_free(&triangles);
}
