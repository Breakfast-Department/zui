#include <zui/internal/widget_internal.h>
#include <zui/internal/window_internal.h>
#include <zui/internal/icon_internal.h>
#include <zui/internal/font_internal.h>
#include <stdlib.h>
#include <string.h>

extern ZuiRenderer *zui_get_renderer(void);

extern void zui_window_close(ZuiWindow *window);
extern void zui_window_minimize(ZuiWindow *window);
extern void zui_window_maximize(ZuiWindow *window);

struct ZuiButton {
    ZuiWidget base;
    char *text;
    ZuiColor normal_color;
    ZuiColor hover_color;
    ZuiColor pressed_color;
};

typedef struct ZuiIconButton {
    ZuiButton base;
    ZuiIconSource *icon_source;
    ZuiTexture icon_texture;
    ZuiColor icon_color;
    float icon_size;
} ZuiIconButton;

typedef struct ZuiMaximizeButton {
    ZuiButton base;
    ZuiIconSource *maximize_icon;
    ZuiIconSource *restore_icon;
    ZuiTexture maximize_texture;
    ZuiTexture restore_texture;
    ZuiColor icon_color;
    float icon_size;
    ZuiWindow *window;
} ZuiMaximizeButton;

struct ZuiLabel {
    ZuiWidget base;
    char *text;
    ZuiColor text_color;
    ZuiFont *font;
    bool owns_font;
};

static void button_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
    ZuiButton *button = (ZuiButton *)widget;

    ZuiColor color = button->normal_color;
    if (widget->pressed) {
        color = button->pressed_color;
    } else if (widget->hovered) {
        color = button->hover_color;
    }

    zui_renderer_draw_rounded_rect(renderer,
        ZUI_RECT(widget->bounds.x, widget->bounds.y,
                  widget->bounds.width, widget->bounds.height),
        color, widget->corner_radius);
}

static bool button_hit_test(ZuiWidget *widget, float x, float y)
{
    (void)x;
    (void)y;
    return true;
}

static void button_on_mouse_enter(ZuiWidget *widget)
{
    (void)widget;
}

static void button_on_mouse_leave(ZuiWidget *widget)
{
    (void)widget;
}

static void button_destroy(ZuiWidget *widget)
{
    ZuiButton *button = (ZuiButton *)widget;
    free(button->text);
}

static const ZuiWidgetVTable button_vtable = {
    .draw = button_draw,
    .hit_test = button_hit_test,
    .on_mouse_enter = button_on_mouse_enter,
    .on_mouse_leave = button_on_mouse_leave,
    .destroy = button_destroy,
};

static void icon_button_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
    ZuiIconButton *btn = (ZuiIconButton *)widget;

    ZuiColor color = btn->base.normal_color;
    if (widget->pressed) {
        color = btn->base.pressed_color;
    } else if (widget->hovered) {
        color = btn->base.hover_color;
    }

    zui_renderer_draw_rounded_rect(renderer,
        ZUI_RECT(widget->bounds.x, widget->bounds.y,
                  widget->bounds.width, widget->bounds.height),
        color, widget->corner_radius);

    if (btn->icon_source && btn->icon_texture.id) {
        float icon_x = widget->bounds.x + (widget->bounds.width - btn->icon_size) / 2;
        float icon_y = widget->bounds.y + (widget->bounds.height - btn->icon_size) / 2;
        zui_renderer_draw_texture(renderer, &btn->icon_texture,
                                   ZUI_RECT(icon_x, icon_y, btn->icon_size, btn->icon_size),
                                   btn->icon_color);
    }
}

static void icon_button_destroy(ZuiWidget *widget)
{
    ZuiIconButton *btn = (ZuiIconButton *)widget;
    if (btn->icon_texture.id) {
        zui_texture_destroy(&btn->icon_texture);
    }
    if (btn->icon_source) {
        zui_icon_source_destroy(btn->icon_source);
    }
    button_destroy(widget);
}

static const ZuiWidgetVTable icon_button_vtable = {
    .draw = icon_button_draw,
    .hit_test = button_hit_test,
    .on_mouse_enter = button_on_mouse_enter,
    .on_mouse_leave = button_on_mouse_leave,
    .destroy = icon_button_destroy,
};

extern bool zui_window_is_maximized(ZuiWindow *window);

static void maximize_button_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
    ZuiMaximizeButton *btn = (ZuiMaximizeButton *)widget;

    ZuiColor color = btn->base.normal_color;
    if (widget->pressed) {
        color = btn->base.pressed_color;
    } else if (widget->hovered) {
        color = btn->base.hover_color;
    }

    zui_renderer_draw_rounded_rect(renderer,
        ZUI_RECT(widget->bounds.x, widget->bounds.y,
                  widget->bounds.width, widget->bounds.height),
        color, widget->corner_radius);

    ZuiTexture *tex = zui_window_is_maximized(btn->window)
                      ? &btn->restore_texture
                      : &btn->maximize_texture;

    if (tex && tex->id) {
        float icon_x = widget->bounds.x + (widget->bounds.width - btn->icon_size) / 2;
        float icon_y = widget->bounds.y + (widget->bounds.height - btn->icon_size) / 2;
        zui_renderer_draw_texture(renderer, tex,
                                   ZUI_RECT(icon_x, icon_y, btn->icon_size, btn->icon_size),
                                   btn->icon_color);
    }
}

static void maximize_button_destroy(ZuiWidget *widget)
{
    ZuiMaximizeButton *btn = (ZuiMaximizeButton *)widget;
    if (btn->maximize_texture.id) zui_texture_destroy(&btn->maximize_texture);
    if (btn->restore_texture.id) zui_texture_destroy(&btn->restore_texture);
    if (btn->maximize_icon) zui_icon_source_destroy(btn->maximize_icon);
    if (btn->restore_icon) zui_icon_source_destroy(btn->restore_icon);
    button_destroy(widget);
}

static const ZuiWidgetVTable maximize_button_vtable = {
    .draw = maximize_button_draw,
    .hit_test = button_hit_test,
    .on_mouse_enter = button_on_mouse_enter,
    .on_mouse_leave = button_on_mouse_leave,
    .destroy = maximize_button_destroy,
};

static ZuiIconButton *create_icon_button(const char *svg_path, float icon_size,
                                          ZuiColor icon_color)
{
    ZuiIconButton *btn = (ZuiIconButton *)zui_widget_create(
        sizeof(ZuiIconButton), ZUI_WIDGET_BUTTON, &icon_button_vtable);
    if (!btn) return NULL;

    btn->base.text = NULL;
    btn->base.normal_color = ZUI_COLOR_HEX(0x3d3d3d);
    btn->base.hover_color = ZUI_COLOR_HEX(0x4d4d4d);
    btn->base.pressed_color = ZUI_COLOR_HEX(0x2d2d2d);
    btn->base.base.corner_radius = 6.0f;
    btn->base.base.preferred_size.width = 20.0f;
    btn->base.base.preferred_size.height = 20.0f;
    btn->base.base.cursor = ZUI_CURSOR_POINTER;

    btn->icon_size = icon_size;
    btn->icon_color = icon_color;
    btn->icon_source = zui_icon_load_svg(svg_path);

    if (btn->icon_source) {
        btn->icon_texture = zui_texture_create(btn->icon_source->raster_data,
                                                btn->icon_source->raster_width,
                                                btn->icon_source->raster_height);
    }

    return btn;
}

ZuiButton *zui_button_create(const char *text)
{
    ZuiButton *button = (ZuiButton *)zui_widget_create(
        sizeof(ZuiButton), ZUI_WIDGET_BUTTON, &button_vtable);
    if (!button) return NULL;

    button->text = text ? strdup(text) : NULL;
    button->normal_color = ZUI_COLOR_HEX(0x3d3d3d);
    button->hover_color = ZUI_COLOR_HEX(0x4d4d4d);
    button->pressed_color = ZUI_COLOR_HEX(0x2d2d2d);

    button->base.corner_radius = 6.0f;
    button->base.preferred_size.width = 80.0f;
    button->base.preferred_size.height = 32.0f;
    button->base.padding = 12.0f;
    button->base.cursor = ZUI_CURSOR_POINTER;

    return button;
}

void zui_button_set_text(ZuiButton *button, const char *text)
{
    if (!button) return;
    free(button->text);
    button->text = text ? strdup(text) : NULL;
}

void zui_button_set_size(ZuiButton *button, float width, float height)
{
    if (!button) return;
    button->base.preferred_size.width = width;
    button->base.preferred_size.height = height;
}

void zui_button_set_color(ZuiButton *button, ZuiColor color)
{
    if (!button) return;
    button->normal_color = color;
    button->hover_color = ZUI_COLOR(color.r * 1.2f, color.g * 1.2f,
                                     color.b * 1.2f, color.a);
    button->pressed_color = ZUI_COLOR(color.r * 0.8f, color.g * 0.8f,
                                       color.b * 0.8f, color.a);
}

void zui_button_on_click(ZuiButton *button, ZuiClickCallback callback,
                          void *user_data)
{
    if (!button) return;
    button->base.on_click = callback;
    button->base.user_data = user_data;
}

static void label_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
    ZuiLabel *label = (ZuiLabel *)widget;
    if (!label->text || !label->font) return;

    float text_w = zui_font_text_width(label->font, label->text);
    float text_h = zui_font_line_height(label->font);

    float x = widget->bounds.x + (widget->bounds.width - text_w) / 2;
    float y = widget->bounds.y + (widget->bounds.height - text_h) / 2;

    zui_font_render_text(label->font, renderer, x, y, label->text, label->text_color);
}

static void label_destroy(ZuiWidget *widget)
{
    ZuiLabel *label = (ZuiLabel *)widget;
    free(label->text);
    if (label->owns_font && label->font) {
        zui_font_destroy(label->font);
    }
}

static const ZuiWidgetVTable label_vtable = {
    .draw = label_draw,
    .destroy = label_destroy,
};

static ZuiFont *g_default_font = NULL;

static ZuiFont *get_default_font(void)
{
    if (!g_default_font) {
        const char *font_paths[] = {
            "/usr/share/fonts/noto/NotoSans-Regular.ttf",
            "/usr/share/fonts/TTF/NotoSans-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        };
        for (size_t i = 0; i < sizeof(font_paths) / sizeof(font_paths[0]); i++) {
            g_default_font = zui_font_load(font_paths[i], 14.0f);
            if (g_default_font) break;
        }
    }
    return g_default_font;
}

ZuiLabel *zui_label_create(const char *text)
{
    ZuiLabel *label = (ZuiLabel *)zui_widget_create(
        sizeof(ZuiLabel), ZUI_WIDGET_LABEL, &label_vtable);
    if (!label) return NULL;

    label->text = text ? strdup(text) : NULL;
    label->text_color = ZUI_COLOR_HEX(0xffffff);
    label->font = get_default_font();
    label->owns_font = false;

    if (label->font && text) {
        label->base.preferred_size.width = zui_font_text_width(label->font, text) + 16;
        label->base.preferred_size.height = zui_font_line_height(label->font) + 8;
    } else {
        size_t len = text ? strlen(text) : 0;
        label->base.preferred_size.width = (float)(len * 8 + 16);
        label->base.preferred_size.height = 24.0f;
    }

    return label;
}

void zui_label_set_text(ZuiLabel *label, const char *text)
{
    if (!label) return;
    free(label->text);
    label->text = text ? strdup(text) : NULL;

    if (label->font && text) {
        label->base.preferred_size.width = zui_font_text_width(label->font, text) + 16;
    } else {
        size_t len = text ? strlen(text) : 0;
        label->base.preferred_size.width = (float)(len * 8 + 16);
    }
}

void zui_label_set_font(ZuiLabel *label, ZuiFont *font)
{
    if (!label) return;
    if (label->owns_font && label->font) {
        zui_font_destroy(label->font);
    }
    label->font = font;
    label->owns_font = false;

    if (label->font && label->text) {
        label->base.preferred_size.width = zui_font_text_width(label->font, label->text) + 16;
        label->base.preferred_size.height = zui_font_line_height(label->font) + 8;
    }
}

void zui_label_set_color(ZuiLabel *label, ZuiColor color)
{
    if (!label) return;
    label->text_color = color;
}

static void close_button_click(ZuiWidget *widget, void *user_data)
{
    ZuiWindow *window = user_data;
    zui_window_close(window);
}

static void minimize_button_click(ZuiWidget *widget, void *user_data)
{
    (void)widget;
    ZuiWindow *window = user_data;
    zui_window_minimize(window);
}

static void hide_button_click(ZuiWidget *widget, void *user_data)
{
    (void)widget;
    ZuiWindow *window = user_data;
    zui_window_minimize(window);
}

static void maximize_button_click(ZuiWidget *widget, void *user_data)
{
    ZuiWindow *window = user_data;
    zui_window_maximize(window);
}

ZuiWidget *zui_window_close_button(ZuiWindow *window)
{
    ZuiIconButton *btn = create_icon_button("assets/icons/x-exit-icon.svg",
                                             12.0f, ZUI_COLOR_RGB(1.0f, 1.0f, 1.0f));
    if (!btn) return NULL;

    btn->base.normal_color = ZUI_COLOR_HEX(0xe81123);
    btn->base.hover_color = ZUI_COLOR_HEX(0xf1707a);
    btn->base.pressed_color = ZUI_COLOR_HEX(0xc42b1c);

    btn->base.base.preferred_size.width = 20.0f;
    btn->base.base.preferred_size.height = 20.0f;
    btn->base.base.corner_radius = 10.0f;

    zui_button_on_click(&btn->base, close_button_click, window);
    return (ZuiWidget *)btn;
}

ZuiWidget *zui_window_minimize_button(ZuiWindow *window)
{
    ZuiIconButton *btn = create_icon_button("assets/icons/x-minimize-icon.svg",
                                             12.0f, ZUI_COLOR_RGB(0.9f, 0.9f, 0.9f));
    if (!btn) return NULL;

    btn->base.normal_color = ZUI_COLOR_HEX(0x3d3d3d);
    btn->base.hover_color = ZUI_COLOR_HEX(0x4d4d4d);
    btn->base.pressed_color = ZUI_COLOR_HEX(0x2d2d2d);

    btn->base.base.preferred_size.width = 20.0f;
    btn->base.base.preferred_size.height = 20.0f;
    btn->base.base.corner_radius = 10.0f;

    zui_button_on_click(&btn->base, minimize_button_click, window);
    return (ZuiWidget *)btn;
}

ZuiWidget *zui_window_maximize_button(ZuiWindow *window)
{
    ZuiMaximizeButton *btn = (ZuiMaximizeButton *)zui_widget_create(
        sizeof(ZuiMaximizeButton), ZUI_WIDGET_BUTTON, &maximize_button_vtable);
    if (!btn) return NULL;

    btn->base.text = NULL;
    btn->base.normal_color = ZUI_COLOR_HEX(0x3d3d3d);
    btn->base.hover_color = ZUI_COLOR_HEX(0x4d4d4d);
    btn->base.pressed_color = ZUI_COLOR_HEX(0x2d2d2d);

    btn->base.base.preferred_size.width = 20.0f;
    btn->base.base.preferred_size.height = 20.0f;
    btn->base.base.corner_radius = 10.0f;
    btn->base.base.cursor = ZUI_CURSOR_POINTER;

    btn->icon_size = 12.0f;
    btn->icon_color = ZUI_COLOR_RGB(1.0f, 1.0f, 1.0f);
    btn->window = window;

    btn->maximize_icon = zui_icon_load_svg("assets/icons/x-maximize-icon.svg");
    btn->restore_icon = zui_icon_load_svg("assets/icons/x-minimize-icon.svg");

    if (btn->maximize_icon) {
        btn->maximize_texture = zui_texture_create(btn->maximize_icon->raster_data,
                                                    btn->maximize_icon->raster_width,
                                                    btn->maximize_icon->raster_height);
    }
    if (btn->restore_icon) {
        btn->restore_texture = zui_texture_create(btn->restore_icon->raster_data,
                                                   btn->restore_icon->raster_width,
                                                   btn->restore_icon->raster_height);
    }

    zui_button_on_click(&btn->base, maximize_button_click, window);
    return (ZuiWidget *)btn;
}

ZuiWidget *zui_window_hide_button(ZuiWindow *window)
{
    ZuiIconButton *btn = create_icon_button("assets/icons/x-hidden-icon.svg",
                                             12.0f, ZUI_COLOR_RGB(1.0f, 1.0f, 1.0f));
    if (!btn) return NULL;

    btn->base.normal_color = ZUI_COLOR_HEX(0x3d3d3d);
    btn->base.hover_color = ZUI_COLOR_HEX(0x4d4d4d);
    btn->base.pressed_color = ZUI_COLOR_HEX(0x2d2d2d);

    btn->base.base.preferred_size.width = 20.0f;
    btn->base.base.preferred_size.height = 20.0f;
    btn->base.base.corner_radius = 10.0f;

    zui_button_on_click(&btn->base, hide_button_click, window);
    return (ZuiWidget *)btn;
}
