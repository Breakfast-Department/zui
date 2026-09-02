#ifndef ZUI_WAYLAND_PLATFORM_H
#define ZUI_WAYLAND_PLATFORM_H

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <stdbool.h>
#include <stdint.h>

#include "protocols/xdg-shell-client-protocol.h"
#include "protocols/xdg-decoration-unstable-v1-client-protocol.h"

typedef struct ZuiWindow ZuiWindow;

typedef enum ZuiCursor {
    ZUI_CURSOR_DEFAULT,
    ZUI_CURSOR_POINTER,
    ZUI_CURSOR_TEXT,
    ZUI_CURSOR_CROSSHAIR,
    ZUI_CURSOR_MOVE,
    ZUI_CURSOR_RESIZE_N,
    ZUI_CURSOR_RESIZE_S,
    ZUI_CURSOR_RESIZE_E,
    ZUI_CURSOR_RESIZE_W,
    ZUI_CURSOR_RESIZE_NE,
    ZUI_CURSOR_RESIZE_NW,
    ZUI_CURSOR_RESIZE_SE,
    ZUI_CURSOR_RESIZE_SW,
    ZUI_CURSOR_NOT_ALLOWED,
    ZUI_CURSOR_GRAB,
    ZUI_CURSOR_GRABBING,
    ZUI_CURSOR_COUNT
} ZuiCursor;

typedef struct ZuiPlatform {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct xdg_wm_base *xdg_wm_base;
    struct zxdg_decoration_manager_v1 *decoration_manager;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;

    struct wl_cursor_theme *cursor_theme;
    struct wl_surface *cursor_surface;
    struct wl_cursor *cursors[ZUI_CURSOR_COUNT];
    ZuiCursor current_cursor;

    ZuiWindow *pointer_focus;
    ZuiWindow *keyboard_focus;

    double pointer_x;
    double pointer_y;
    uint32_t pointer_button;
    uint32_t pointer_serial;
    uint32_t keyboard_serial;

    bool running;
} ZuiPlatform;

typedef struct ZuiWaylandWindow {
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct zxdg_toplevel_decoration_v1 *decoration;
    struct wl_egl_window *egl_window;
    struct wl_callback *frame_callback;

    bool configured;
    bool pending_resize;
    int pending_width;
    int pending_height;
} ZuiWaylandWindow;

bool zui_platform_init(ZuiPlatform *platform);
void zui_platform_shutdown(ZuiPlatform *platform);
void zui_platform_poll_events(ZuiPlatform *platform);

bool zui_wayland_window_create(ZuiPlatform *platform, ZuiWaylandWindow *wl_win,
                                ZuiWindow *window, int width, int height,
                                const char *title);
void zui_wayland_window_destroy(ZuiPlatform *platform, ZuiWaylandWindow *wl_win);
void zui_wayland_window_set_title(ZuiWaylandWindow *wl_win, const char *title);
void zui_wayland_window_request_frame(ZuiWaylandWindow *wl_win, ZuiWindow *window);

void zui_wayland_start_move(ZuiPlatform *platform, ZuiWaylandWindow *wl_win);
void zui_wayland_start_resize(ZuiPlatform *platform, ZuiWaylandWindow *wl_win,
                               uint32_t edges);
void zui_wayland_minimize(ZuiWaylandWindow *wl_win);
void zui_wayland_maximize(ZuiWaylandWindow *wl_win);
void zui_wayland_unmaximize(ZuiWaylandWindow *wl_win);
void zui_wayland_set_min_size(ZuiWaylandWindow *wl_win, int width, int height);
void zui_wayland_set_max_size(ZuiWaylandWindow *wl_win, int width, int height);

void zui_platform_set_cursor(ZuiPlatform *platform, ZuiCursor cursor);

#endif
