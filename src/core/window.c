#include <zui/internal/window_internal.h>
#include <zui/image.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input-event-codes.h>

#define ZUI_DEFAULT_LOGO_PATH "assets/logo/zui-logo-white.png"
#define ZUI_DEFAULT_LOGO_SIZE 14.0f
#define ZUI_DEFAULT_MIN_WIDTH 200
#define ZUI_DEFAULT_MIN_HEIGHT 35
#define ZUI_RESIZE_BORDER 6.0f
#define ZUI_RESIZE_CORNER 12.0f

extern ZuiPlatform *zui_get_platform(void);
extern ZuiEglContext *zui_get_egl(void);
extern ZuiRenderer *zui_get_renderer(void);
extern bool zui_init_renderer_if_needed(void);

static ZuiWidget *g_hovered_widget = NULL;
static ZuiWidget *g_pressed_widget = NULL;
static uint32_t g_resize_edge = 0;

static uint32_t detect_resize_edge(ZuiWindow *window, float x, float y)
{
    if (window->maximized) return 0;

    float w = (float)window->width;
    float h = (float)window->height;
    float border = ZUI_RESIZE_BORDER;
    float corner = ZUI_RESIZE_CORNER;

    bool left = x < border;
    bool right = x > w - border;
    bool top = y < border;
    bool bottom = y > h - border;

    bool corner_left = x < corner;
    bool corner_right = x > w - corner;
    bool corner_top = y < corner;
    bool corner_bottom = y > h - corner;

    if (corner_top && corner_left) return 5;
    if (corner_top && corner_right) return 9;
    if (corner_bottom && corner_left) return 6;
    if (corner_bottom && corner_right) return 10;

    if (top) return 1;
    if (bottom) return 2;
    if (left) return 4;
    if (right) return 8;

    return 0;
}

static ZuiCursor resize_edge_to_cursor(uint32_t edge)
{
    switch (edge) {
        case 1: return ZUI_CURSOR_RESIZE_N;
        case 2: return ZUI_CURSOR_RESIZE_S;
        case 4: return ZUI_CURSOR_RESIZE_W;
        case 8: return ZUI_CURSOR_RESIZE_E;
        case 5: return ZUI_CURSOR_RESIZE_NW;
        case 9: return ZUI_CURSOR_RESIZE_NE;
        case 6: return ZUI_CURSOR_RESIZE_SW;
        case 10: return ZUI_CURSOR_RESIZE_SE;
        default: return ZUI_CURSOR_DEFAULT;
    }
}

static void titlebar_layout(ZuiWidget *widget);
static void window_layout(ZuiWidget *widget);

static const ZuiWidgetVTable titlebar_vtable = {
    .layout = titlebar_layout,
};

static const ZuiWidgetVTable window_vtable = {
    .layout = window_layout,
};

static void container_layout(ZuiWidget *widget)
{
    float x = widget->bounds.x + widget->padding;
    float y = widget->bounds.y + widget->padding;
    float available_w = widget->bounds.width - widget->padding * 2;
    float available_h = widget->bounds.height - widget->padding * 2;

    if (widget->layout_dir == ZUI_LAYOUT_HORIZONTAL) {
        float total_width = 0;
        for (int i = 0; i < widget->child_count; i++) {
            ZuiWidget *child = widget->children[i];
            if (!child->visible) continue;
            total_width += child->preferred_size.width;
            if (i > 0) total_width += widget->spacing;
        }

        float start_x = x;
        if (widget->align == ZUI_ALIGN_CENTER) {
            start_x = x + (available_w - total_width) / 2;
        } else if (widget->align == ZUI_ALIGN_END) {
            start_x = x + available_w - total_width;
        }

        float cx = start_x;
        for (int i = 0; i < widget->child_count; i++) {
            ZuiWidget *child = widget->children[i];
            if (!child->visible) continue;

            float ch = child->expand ? available_h : child->preferred_size.height;
            float cy = y + (available_h - ch) / 2;

            zui_widget_set_bounds(child, cx, cy,
                                   child->preferred_size.width, ch);
            cx += child->preferred_size.width + widget->spacing;
        }
    } else {
        float cy = y;
        for (int i = 0; i < widget->child_count; i++) {
            ZuiWidget *child = widget->children[i];
            if (!child->visible) continue;

            float cw = child->expand ? available_w : child->preferred_size.width;
            zui_widget_set_bounds(child, x, cy, cw, child->preferred_size.height);
            cy += child->preferred_size.height + widget->spacing;
        }
    }
}

static const ZuiWidgetVTable container_vtable = {
    .layout = container_layout,
};

static void titlebar_layout(ZuiWidget *widget)
{
    ZuiTitlebar *titlebar = (ZuiTitlebar *)widget;
    float x = widget->bounds.x;
    float y = widget->bounds.y;
    float w = widget->bounds.width;
    float h = widget->bounds.height;
    float padding = widget->padding;

    float start_w = 0, center_w = 0, end_w = 0;

    if (titlebar->start_container) {
        for (int i = 0; i < titlebar->start_container->child_count; i++) {
            ZuiWidget *child = titlebar->start_container->children[i];
            start_w += child->preferred_size.width;
            if (i > 0) start_w += titlebar->start_container->spacing;
        }
        start_w += titlebar->start_container->padding * 2;
    }

    if (titlebar->center_container) {
        for (int i = 0; i < titlebar->center_container->child_count; i++) {
            ZuiWidget *child = titlebar->center_container->children[i];
            center_w += child->preferred_size.width;
            if (i > 0) center_w += titlebar->center_container->spacing;
        }
        center_w += titlebar->center_container->padding * 2;
    }

    if (titlebar->end_container) {
        for (int i = 0; i < titlebar->end_container->child_count; i++) {
            ZuiWidget *child = titlebar->end_container->children[i];
            end_w += child->preferred_size.width;
            if (i > 0) end_w += titlebar->end_container->spacing;
        }
        end_w += titlebar->end_container->padding * 2;
    }

    float max_side = start_w > end_w ? start_w : end_w;

    if (titlebar->start_container) {
        zui_widget_set_bounds((ZuiWidget *)titlebar->start_container,
                               x + padding, y, start_w, h);
        container_layout(titlebar->start_container);
    }

    if (titlebar->center_container) {
        float center_x = x + (w - center_w) / 2;
        float min_center_x = x + max_side + padding;
        float max_center_x = x + w - max_side - center_w - padding;
        if (center_x < min_center_x) center_x = min_center_x;
        if (center_x > max_center_x) center_x = max_center_x;
        zui_widget_set_bounds((ZuiWidget *)titlebar->center_container,
                               center_x, y, center_w, h);
        container_layout(titlebar->center_container);
    }

    if (titlebar->end_container) {
        zui_widget_set_bounds((ZuiWidget *)titlebar->end_container,
                               x + w - end_w - padding, y, end_w, h);
        container_layout(titlebar->end_container);
    }
}

static void window_layout(ZuiWidget *widget)
{
    ZuiWindow *window = (ZuiWindow *)widget;
    float x = widget->bounds.x;
    float y = widget->bounds.y;
    float w = widget->bounds.width;

    float titlebar_h = window->titlebar ? window->titlebar->height : 0;

    if (window->titlebar) {
        zui_widget_set_bounds((ZuiWidget *)window->titlebar, x, y, w, titlebar_h);
        zui_widget_layout((ZuiWidget *)window->titlebar);
    }

    if (window->content) {
        float content_y = y + titlebar_h;
        float content_h = widget->bounds.height - titlebar_h;
        zui_widget_set_bounds(window->content, x, content_y, w, content_h);
        zui_widget_layout(window->content);
    }
}

static ZuiWidget *create_container(void)
{
    ZuiWidget *container = zui_widget_create(sizeof(ZuiWidget),
                                              ZUI_WIDGET_CONTAINER,
                                              &container_vtable);
    if (!container) return NULL;

    container->layout_dir = ZUI_LAYOUT_HORIZONTAL;
    container->align = ZUI_ALIGN_CENTER;
    container->spacing = 8.0f;
    container->padding = 8.0f;

    return container;
}

ZuiTitlebar *zui_titlebar_create(float height)
{
    ZuiTitlebar *titlebar = (ZuiTitlebar *)zui_widget_create(
        sizeof(ZuiTitlebar), ZUI_WIDGET_TITLEBAR, &titlebar_vtable);
    if (!titlebar) return NULL;

    titlebar->height = height;
    titlebar->base.padding = 2.0f;
    titlebar->logo = NULL;

    titlebar->start_container = create_container();
    titlebar->center_container = create_container();
    titlebar->end_container = create_container();

    if (titlebar->start_container) {
        titlebar->start_container->align = ZUI_ALIGN_START;
        titlebar->start_container->padding = 4.0f;
        zui_widget_add_child((ZuiWidget *)titlebar, titlebar->start_container);
    }
    if (titlebar->center_container) {
        titlebar->center_container->align = ZUI_ALIGN_CENTER;
        zui_widget_add_child((ZuiWidget *)titlebar, titlebar->center_container);
    }
    if (titlebar->end_container) {
        titlebar->end_container->align = ZUI_ALIGN_END;
        titlebar->end_container->padding = 4.0f;
        zui_widget_add_child((ZuiWidget *)titlebar, titlebar->end_container);
    }

    titlebar->logo = zui_image_create(ZUI_DEFAULT_LOGO_PATH);
    if (titlebar->logo) {
        zui_image_set_size(titlebar->logo, ZUI_DEFAULT_LOGO_SIZE, ZUI_DEFAULT_LOGO_SIZE);
        if (titlebar->start_container) {
            zui_widget_add_child(titlebar->start_container, zui_image_widget(titlebar->logo));
        }
    }

    return titlebar;
}

void zui_titlebar_set_start(ZuiTitlebar *titlebar, ZuiWidget *widget)
{
    if (!titlebar || !widget || !titlebar->start_container) return;
    zui_widget_add_child(titlebar->start_container, widget);
}

void zui_titlebar_set_center(ZuiTitlebar *titlebar, ZuiWidget *widget)
{
    if (!titlebar || !widget || !titlebar->center_container) return;
    zui_widget_add_child(titlebar->center_container, widget);
}

void zui_titlebar_set_end(ZuiTitlebar *titlebar, ZuiWidget *widget)
{
    if (!titlebar || !widget || !titlebar->end_container) return;
    zui_widget_add_child(titlebar->end_container, widget);
}

ZuiWindow *zui_window_create(int width, int height, const char *title)
{
    ZuiPlatform *platform = zui_get_platform();
    ZuiEglContext *egl = zui_get_egl();

    ZuiWindow *window = (ZuiWindow *)zui_widget_create(
        sizeof(ZuiWindow), ZUI_WIDGET_WINDOW, &window_vtable);
    if (!window) return NULL;

    window->title = title ? strdup(title) : NULL;
    window->width = width;
    window->height = height;
    window->corner_radius = 12.0f;
    window->running = true;
    window->active = true;
    window->needs_redraw = true;

    window->base.bounds.width = (float)width;
    window->base.bounds.height = (float)height;
    window->base.background = ZUI_COLOR(0, 0, 0, 0);
    window->base.corner_radius = 0;
    window->background_color = ZUI_COLOR_HEX(0x242424);
    window->border_color = ZUI_COLOR_HEX(0x3d3d3d);
    window->border_width = 1.0f;

    if (!zui_wayland_window_create(platform, &window->wayland, window,
                                    width, height, title ? title : "ZUI")) {
        free(window->title);
        free(window);
        return NULL;
    }

    if (!zui_egl_surface_create(egl, &window->egl_surface,
                                 window->wayland.egl_window)) {
        zui_wayland_window_destroy(platform, &window->wayland);
        free(window->title);
        free(window);
        return NULL;
    }

    window->min_width = ZUI_DEFAULT_MIN_WIDTH;
    window->min_height = ZUI_DEFAULT_MIN_HEIGHT;
    zui_wayland_set_min_size(&window->wayland, ZUI_DEFAULT_MIN_WIDTH, ZUI_DEFAULT_MIN_HEIGHT);

    zui_egl_make_current(egl, &window->egl_surface);
    zui_init_renderer_if_needed();

    window->titlebar = zui_titlebar_create(32.0f);
    if (window->titlebar) {
        window->titlebar->base.background = ZUI_COLOR(0, 0, 0, 0);
        window->titlebar->base.corner_radius = window->corner_radius;
        window->titlebar->base.corner_mode = ZUI_CORNERS_TOP;
        zui_widget_add_child((ZuiWidget *)window, (ZuiWidget *)window->titlebar);
    }

    window->content = create_container();
    if (window->content) {
        window->content->layout_dir = ZUI_LAYOUT_VERTICAL;
        window->content->align = ZUI_ALIGN_START;
        window->content->padding = 0.0f;
        window->content->spacing = 0.0f;
        zui_widget_add_child((ZuiWidget *)window, window->content);
    }

    return window;
}

void zui_window_destroy(ZuiWindow *window)
{
    if (!window) return;

    ZuiPlatform *platform = zui_get_platform();
    ZuiEglContext *egl = zui_get_egl();

    zui_egl_surface_destroy(egl, &window->egl_surface);
    zui_wayland_window_destroy(platform, &window->wayland);

    free(window->title);
    zui_widget_destroy((ZuiWidget *)window);
}

void zui_window_show(ZuiWindow *window)
{
    if (!window) return;
    window->needs_redraw = true;
}

bool zui_window_running(ZuiWindow *window)
{
    return window && window->running;
}

void zui_window_render(ZuiWindow *window)
{
    if (!window || !window->needs_redraw) return;

    ZuiEglContext *egl = zui_get_egl();
    ZuiRenderer *renderer = zui_get_renderer();

    zui_egl_make_current(egl, &window->egl_surface);

    float radius = window->maximized ? 0 : window->corner_radius;
    float border = window->maximized ? 0 : window->border_width;
    float inset = border;
    float inner_radius = radius > inset ? radius - inset : 0;

    zui_widget_set_bounds((ZuiWidget *)window, inset, inset,
                           (float)window->width - inset * 2,
                           (float)window->height - inset * 2);
    zui_widget_layout((ZuiWidget *)window);

    zui_renderer_begin(renderer, window->width, window->height);
    zui_renderer_clear(renderer, ZUI_COLOR(0, 0, 0, 0));

    if (border > 0 && !window->maximized) {
        zui_renderer_draw_rounded_rect_outline(renderer,
            ZUI_RECT(0, 0, (float)window->width, (float)window->height),
            window->border_color, radius, border);
    }

    zui_renderer_push_clip(renderer,
        ZUI_RECT(inset, inset,
                 (float)window->width - inset * 2,
                 (float)window->height - inset * 2),
        inner_radius);

    zui_renderer_draw_rounded_rect(renderer,
        ZUI_RECT(inset, inset,
                 (float)window->width - inset * 2,
                 (float)window->height - inset * 2),
        window->background_color, inner_radius);

    zui_widget_draw((ZuiWidget *)window, renderer);

    zui_renderer_pop_clip(renderer);

    zui_renderer_end(renderer);
    zui_egl_swap_buffers(egl, &window->egl_surface);

    window->needs_redraw = false;

    zui_wayland_window_request_frame(&window->wayland, window);
}

void zui_window_set_title(ZuiWindow *window, const char *title)
{
    if (!window) return;

    free(window->title);
    window->title = title ? strdup(title) : NULL;
    zui_wayland_window_set_title(&window->wayland, title ? title : "ZUI");
}

void zui_window_set_corner_radius(ZuiWindow *window, float radius)
{
    if (!window) return;
    window->corner_radius = radius;
    window->base.corner_radius = radius;
    window->needs_redraw = true;
}

void zui_window_set_min_size(ZuiWindow *window, int width, int height)
{
    if (!window) return;
    window->min_width = width;
    window->min_height = height;
    zui_wayland_set_min_size(&window->wayland, width, height);
}

void zui_window_set_max_size(ZuiWindow *window, int width, int height)
{
    if (!window) return;
    window->max_width = width;
    window->max_height = height;
    zui_wayland_set_max_size(&window->wayland, width, height);
}

ZuiTitlebar *zui_window_titlebar(ZuiWindow *window)
{
    return window ? window->titlebar : NULL;
}

ZuiWidget *zui_window_content(ZuiWindow *window)
{
    return window ? window->content : NULL;
}

ZuiWaylandWindow *zui_window_get_wayland(ZuiWindow *window)
{
    return window ? &window->wayland : NULL;
}

void zui_window_handle_resize(ZuiWindow *window, int width, int height)
{
    if (!window) return;
    window->width = width;
    window->height = height;
    window->base.bounds.width = (float)width;
    window->base.bounds.height = (float)height;
    window->needs_redraw = true;
}

static void update_hover(ZuiWindow *window, ZuiWidget *new_hover)
{
    if (g_hovered_widget == new_hover) return;

    if (g_hovered_widget) {
        g_hovered_widget->hovered = false;
        if (g_hovered_widget->vtable && g_hovered_widget->vtable->on_mouse_leave) {
            g_hovered_widget->vtable->on_mouse_leave(g_hovered_widget);
        }
        window->needs_redraw = true;
    }

    g_hovered_widget = new_hover;

    if (g_hovered_widget) {
        g_hovered_widget->hovered = true;
        if (g_hovered_widget->vtable && g_hovered_widget->vtable->on_mouse_enter) {
            g_hovered_widget->vtable->on_mouse_enter(g_hovered_widget);
        }
        window->needs_redraw = true;
    }

    ZuiPlatform *platform = zui_get_platform();
    ZuiCursor cursor = new_hover ? new_hover->cursor : ZUI_CURSOR_DEFAULT;
    zui_platform_set_cursor(platform, cursor);
}

void zui_window_handle_motion(ZuiWindow *window, double x, double y)
{
    if (!window) return;

    g_resize_edge = detect_resize_edge(window, (float)x, (float)y);

    if (g_resize_edge) {
        update_hover(window, NULL);
        ZuiPlatform *platform = zui_get_platform();
        zui_platform_set_cursor(platform, resize_edge_to_cursor(g_resize_edge));
    } else {
        ZuiWidget *hit = zui_widget_hit_test((ZuiWidget *)window, (float)x, (float)y);
        update_hover(window, hit);
    }
}

void zui_window_handle_button(ZuiWindow *window, double x, double y,
                               uint32_t button, bool pressed)
{
    if (!window) return;

    if (pressed && button == BTN_LEFT && g_resize_edge) {
        ZuiPlatform *platform = zui_get_platform();
        zui_wayland_start_resize(platform, &window->wayland, g_resize_edge);
        return;
    }

    ZuiWidget *hit = zui_widget_hit_test((ZuiWidget *)window, (float)x, (float)y);

    if (pressed && button == BTN_LEFT) {
        g_pressed_widget = hit;

        if (hit && hit->vtable && hit->vtable->on_mouse_down) {
            hit->vtable->on_mouse_down(hit, (float)x, (float)y, button);
        }

        if (hit) {
            hit->pressed = true;
            window->needs_redraw = true;
        }

        if (hit && (hit->type == ZUI_WIDGET_TITLEBAR ||
                    (hit->parent && hit->parent->type == ZUI_WIDGET_TITLEBAR))) {
            bool on_interactive = (hit->type == ZUI_WIDGET_BUTTON);
            if (!on_interactive && hit->parent) {
                ZuiWidget *p = hit->parent;
                while (p && p->type != ZUI_WIDGET_TITLEBAR) {
                    if (p->type == ZUI_WIDGET_BUTTON) {
                        on_interactive = true;
                        break;
                    }
                    p = p->parent;
                }
            }

            if (!on_interactive) {
                ZuiPlatform *platform = zui_get_platform();
                zui_wayland_start_move(platform, &window->wayland);
            }
        }
    } else if (!pressed && button == BTN_LEFT) {
        if (g_pressed_widget) {
            g_pressed_widget->pressed = false;

            if (g_pressed_widget->vtable && g_pressed_widget->vtable->on_mouse_up) {
                g_pressed_widget->vtable->on_mouse_up(g_pressed_widget,
                                                       (float)x, (float)y, button);
            }

            if (hit == g_pressed_widget && g_pressed_widget->on_click) {
                g_pressed_widget->on_click(g_pressed_widget,
                                            g_pressed_widget->user_data);
            }

            window->needs_redraw = true;
            g_pressed_widget = NULL;
        }
    }
}

void zui_window_handle_key(ZuiWindow *window, uint32_t key, bool pressed)
{
    (void)window;
    (void)key;
    (void)pressed;
}

void zui_window_request_close(ZuiWindow *window)
{
    if (window) window->running = false;
}

void zui_window_set_maximized_state(ZuiWindow *window, bool maximized)
{
    if (window) {
        window->maximized = maximized;
        if (window->titlebar) {
            window->titlebar->base.corner_mode = maximized ? ZUI_CORNERS_NONE : ZUI_CORNERS_TOP;
        }
        window->needs_redraw = true;
    }
}

void zui_window_set_active_state(ZuiWindow *window, bool active)
{
    if (window && window->active != active) {
        window->active = active;
        window->needs_redraw = true;
    }
}

void zui_window_mark_needs_redraw(ZuiWindow *window)
{
    if (window) window->needs_redraw = true;
}

void zui_window_minimize(ZuiWindow *window)
{
    if (window) zui_wayland_minimize(&window->wayland);
}

void zui_window_maximize(ZuiWindow *window)
{
    if (!window) return;
    if (window->maximized) {
        zui_wayland_unmaximize(&window->wayland);
    } else {
        zui_wayland_maximize(&window->wayland);
    }
}

void zui_window_close(ZuiWindow *window)
{
    if (window) window->running = false;
}

bool zui_window_is_maximized(ZuiWindow *window)
{
    return window ? window->maximized : false;
}

void zui_window_set_logo(ZuiWindow *window, const char *path)
{
    if (!window || !window->titlebar) return;

    ZuiTitlebar *titlebar = window->titlebar;

    if (titlebar->logo) {
        zui_widget_remove_child(titlebar->start_container,
                                 zui_image_widget(titlebar->logo));
        zui_image_destroy(titlebar->logo);
        titlebar->logo = NULL;
    }

    if (path) {
        titlebar->logo = zui_image_create(path);
        if (titlebar->logo) {
            zui_image_set_size(titlebar->logo, ZUI_DEFAULT_LOGO_SIZE, ZUI_DEFAULT_LOGO_SIZE);
            if (titlebar->start_container) {
                ZuiWidget *logo_widget = zui_image_widget(titlebar->logo);
                if (titlebar->start_container->child_count > 0) {
                    for (int i = titlebar->start_container->child_count - 1; i >= 0; i--) {
                        titlebar->start_container->children[i + 1] =
                            titlebar->start_container->children[i];
                    }
                    titlebar->start_container->children[0] = logo_widget;
                    logo_widget->parent = titlebar->start_container;
                    titlebar->start_container->child_count++;
                } else {
                    zui_widget_add_child(titlebar->start_container, logo_widget);
                }
            }
        }
    }

    window->needs_redraw = true;
}

void zui_window_set_logo_size(ZuiWindow *window, float width, float height)
{
    if (!window || !window->titlebar || !window->titlebar->logo) return;
    zui_image_set_size(window->titlebar->logo, width, height);
    window->needs_redraw = true;
}

void zui_window_set_logo_visible(ZuiWindow *window, bool visible)
{
    if (!window || !window->titlebar || !window->titlebar->logo) return;
    zui_image_set_visible(window->titlebar->logo, visible);
    window->needs_redraw = true;
}

void zui_window_remove_logo(ZuiWindow *window)
{
    if (!window || !window->titlebar) return;

    ZuiTitlebar *titlebar = window->titlebar;

    if (titlebar->logo) {
        zui_widget_remove_child(titlebar->start_container,
                                 zui_image_widget(titlebar->logo));
        zui_image_destroy(titlebar->logo);
        titlebar->logo = NULL;
    }

    window->needs_redraw = true;
}

void zui_window_set_background_color(ZuiWindow *window, ZuiColor color)
{
    if (!window) return;
    window->background_color = color;
    window->needs_redraw = true;
}

void zui_window_set_border_color(ZuiWindow *window, ZuiColor color)
{
    if (!window) return;
    window->border_color = color;
    window->needs_redraw = true;
}

bool zui_window_is_active(ZuiWindow *window)
{
    return window ? window->active : false;
}
