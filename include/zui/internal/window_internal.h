#ifndef ZUI_WINDOW_INTERNAL_H
#define ZUI_WINDOW_INTERNAL_H

#include "widget_internal.h"
#include "wayland_platform.h"
#include "egl_context.h"

typedef struct ZuiImage ZuiImage;

struct ZuiTitlebar {
    ZuiWidget base;
    ZuiWidget *start_container;
    ZuiWidget *center_container;
    ZuiWidget *end_container;
    ZuiImage *logo;
    float height;
};

struct ZuiWindow {
    ZuiWidget base;
    ZuiWaylandWindow wayland;
    ZuiEglSurface egl_surface;

    ZuiTitlebar *titlebar;
    ZuiWidget *content;

    char *title;
    int width;
    int height;
    int min_width;
    int min_height;
    int max_width;
    int max_height;
    float corner_radius;
    ZuiColor background_color;
    ZuiColor border_color;
    float border_width;

    bool running;
    bool maximized;
    bool active;
    bool needs_redraw;
    bool frame_pending;
};

ZuiWaylandWindow *zui_window_get_wayland(ZuiWindow *window);
void zui_window_handle_resize(ZuiWindow *window, int width, int height);
void zui_window_handle_motion(ZuiWindow *window, double x, double y);
void zui_window_handle_button(ZuiWindow *window, double x, double y,
                               uint32_t button, bool pressed);
void zui_window_handle_key(ZuiWindow *window, uint32_t key, bool pressed);
void zui_window_request_close(ZuiWindow *window);
void zui_window_set_maximized_state(ZuiWindow *window, bool maximized);
void zui_window_set_active_state(ZuiWindow *window, bool active);
void zui_window_mark_needs_redraw(ZuiWindow *window);

#endif
