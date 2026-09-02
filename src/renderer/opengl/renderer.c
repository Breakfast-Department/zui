#include <zui/internal/renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "ZUI: Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len < 0) {
        fclose(f);
        return NULL;
    }
    size_t size = (size_t)len;
    fseek(f, 0, SEEK_SET);

    char *content = malloc(size + 1);
    if (!content) {
        fclose(f);
        return NULL;
    }

    if (fread(content, 1, size, f) != size) {
        free(content);
        fclose(f);
        return NULL;
    }

    content[size] = '\0';
    fclose(f);
    return content;
}

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

static GLuint load_shader_program(const char *shader_path,
                                   const char *vert_name,
                                   const char *frag_name)
{
    char vert_path[512], frag_path[512];
    snprintf(vert_path, sizeof(vert_path), "%s/%s", shader_path, vert_name);
    snprintf(frag_path, sizeof(frag_path), "%s/%s", shader_path, frag_name);

    char *vert_src = read_file(vert_path);
    if (!vert_src) return 0;

    char *frag_src = read_file(frag_path);
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

bool zui_renderer_init(ZuiRenderer *renderer, const char *shader_path)
{
    memset(renderer, 0, sizeof(*renderer));
    renderer->clip_stack_top = -1;

    if (!gladLoadGL()) {
        fprintf(stderr, "ZUI: Failed to load OpenGL\n");
        return false;
    }

    renderer->rect_shader = load_shader_program(shader_path,
                                                 "rect.vert", "rect.frag");
    if (!renderer->rect_shader) {
        return false;
    }

    renderer->tex_shader = load_shader_program(shader_path,
                                                "texture.vert", "texture.frag");
    if (!renderer->tex_shader) {
        glDeleteProgram(renderer->rect_shader);
        return false;
    }

    renderer->glyph_shader = load_shader_program(shader_path,
                                                  "glyph.vert", "glyph.frag");
    if (!renderer->glyph_shader) {
        glDeleteProgram(renderer->tex_shader);
        glDeleteProgram(renderer->rect_shader);
        return false;
    }

    renderer->window_shader = load_shader_program(shader_path,
                                                   "window.vert", "window.frag");
    if (!renderer->window_shader) {
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

    glGenVertexArrays(1, &renderer->window_vao);
    glGenBuffers(1, &renderer->window_vbo);

    glBindVertexArray(renderer->window_vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->window_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    return true;
}

void zui_renderer_shutdown(ZuiRenderer *renderer)
{
    if (renderer->window_vbo) glDeleteBuffers(1, &renderer->window_vbo);
    if (renderer->window_vao) glDeleteVertexArrays(1, &renderer->window_vao);
    if (renderer->window_shader) glDeleteProgram(renderer->window_shader);
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

    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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

ZuiTexture zui_texture_create(const uint8_t *data, int width, int height)
{
    ZuiTexture texture = {0, width, height};

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void zui_texture_destroy(ZuiTexture *texture)
{
    if (texture && texture->id) {
        glDeleteTextures(1, &texture->id);
        texture->id = 0;
    }
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

void zui_renderer_push_clip(ZuiRenderer *renderer, ZuiRect rect, float radius)
{
    if (renderer->clip_stack_top >= ZUI_MAX_CLIP_STACK - 1) return;

    ZuiClipState new_clip;
    new_clip.rect = rect;
    new_clip.radius = radius;

    if (renderer->clip_stack_top >= 0 && radius <= 0.0f) {
        ZuiClipState *parent = &renderer->clip_stack[renderer->clip_stack_top];
        if (parent->radius > 0.0f) {
            new_clip.rect = parent->rect;
            new_clip.radius = parent->radius;
        }
    }

    renderer->clip_stack_top++;
    renderer->clip_stack[renderer->clip_stack_top] = new_clip;
}

void zui_renderer_pop_clip(ZuiRenderer *renderer)
{
    if (renderer->clip_stack_top >= 0) {
        renderer->clip_stack_top--;
    }
}

void zui_renderer_draw_window(ZuiRenderer *renderer, ZuiRect rect,
                               const ZuiWindowStyle *style)
{
    glUseProgram(renderer->window_shader);
    glBindVertexArray(renderer->window_vao);

    GLint loc;

    loc = glGetUniformLocation(renderer->window_shader, "u_resolution");
    glUniform2f(loc, (float)renderer->viewport_width,
                (float)renderer->viewport_height);

    loc = glGetUniformLocation(renderer->window_shader, "u_rect_pos");
    glUniform2f(loc, rect.x, rect.y);

    loc = glGetUniformLocation(renderer->window_shader, "u_rect_size");
    glUniform2f(loc, rect.w, rect.h);

    loc = glGetUniformLocation(renderer->window_shader, "u_bg_color");
    glUniform4f(loc, style->bg_color.r, style->bg_color.g,
                style->bg_color.b, style->bg_color.a);

    loc = glGetUniformLocation(renderer->window_shader, "u_border_color");
    glUniform4f(loc, style->border_color.r, style->border_color.g,
                style->border_color.b, style->border_color.a);

    loc = glGetUniformLocation(renderer->window_shader, "u_titlebar_color");
    glUniform4f(loc, style->titlebar_color.r, style->titlebar_color.g,
                style->titlebar_color.b, style->titlebar_color.a);

    loc = glGetUniformLocation(renderer->window_shader, "u_border_width");
    glUniform1f(loc, style->border_width);

    loc = glGetUniformLocation(renderer->window_shader, "u_radius_tl");
    glUniform1f(loc, style->radius_tl);

    loc = glGetUniformLocation(renderer->window_shader, "u_radius_tr");
    glUniform1f(loc, style->radius_tr);

    loc = glGetUniformLocation(renderer->window_shader, "u_radius_br");
    glUniform1f(loc, style->radius_br);

    loc = glGetUniformLocation(renderer->window_shader, "u_radius_bl");
    glUniform1f(loc, style->radius_bl);

    loc = glGetUniformLocation(renderer->window_shader, "u_titlebar_height");
    glUniform1f(loc, style->titlebar_height);

    loc = glGetUniformLocation(renderer->window_shader, "u_curve_intensity");
    glUniform1f(loc, style->curve_intensity);

    loc = glGetUniformLocation(renderer->window_shader, "u_active");
    glUniform1i(loc, style->active ? 1 : 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glUseProgram(0);
}
