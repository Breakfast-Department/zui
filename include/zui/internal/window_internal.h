#ifndef ZUI_WINDOW_INTERNAL_H
#define ZUI_WINDOW_INTERNAL_H

#include "wayland_platform.h"
#include "egl_context.h"
#include "widget_internal.h"

typedef struct ZuiImage ZuiImage;

struct ZuiWindow {
  ZuiWidget base;
  ZuiWaylandWindow wayland;
  ZuiEglSurface egl_surface;

  ZuiWidget *content;
  ZuiWidget *focused;
  ZuiWidget *overlay;
  ZuiWidget *decoration;

  char *title;
  int x;
  int y;
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
  bool minimized;
  bool hidden;
  bool fullscreen;
  bool decorated;
  bool active;
  bool needs_redraw;
  bool frame_pending;

  ZuiWidget **moving_handlers;
  int moving_handler_count;
  int moving_handler_capacity;

  ZuiWidget **close_handlers;
  int close_handler_count;
  int close_handler_capacity;

  ZuiWidget **maximize_handlers;
  int maximize_handler_count;
  int maximize_handler_capacity;

  ZuiWidget **hide_handlers;
  int hide_handler_count;
  int hide_handler_capacity;

  ZuiWidget **minimize_handlers;
  int minimize_handler_count;
  int minimize_handler_capacity;

  void (*custom_draw)(struct ZuiWindow *window, void *user_data);
  void *custom_draw_user_data;
};

ZuiWaylandWindow *zui_window_get_wayland(ZuiWindow *window);
void zui_window_handle_resize(ZuiWindow *window, int width, int height);
void zui_window_handle_motion(ZuiWindow *window, double x, double y);
void zui_window_handle_button(ZuiWindow *window, double x, double y,
                               uint32_t button, bool pressed);
void zui_window_handle_key(ZuiWindow *window, uint32_t key, uint32_t sym,
                            const char *text, bool pressed);
void zui_window_handle_scroll(ZuiWindow *window, double x, double y,
                               double dx, double dy);
void zui_window_request_close(ZuiWindow *window);
void zui_window_set_maximized_state(ZuiWindow *window, bool maximized);
void zui_window_set_fullscreen_state(ZuiWindow *window, bool fullscreen);
void zui_window_set_decorated_state(ZuiWindow *window, bool decorated);
void zui_window_set_active_state(ZuiWindow *window, bool active);
void zui_window_mark_needs_redraw(ZuiWindow *window);

#endif
