#ifndef ZUI_WINDOW_H
#define ZUI_WINDOW_H

#include <stdbool.h>
#include <zui/color.h>

typedef struct ZuiWindow ZuiWindow;
typedef struct ZuiTitlebar ZuiTitlebar;
typedef struct ZuiWidget ZuiWidget;

ZuiWindow *zui_window_create(int width, int height, const char *title);
void zui_window_destroy(ZuiWindow *window);
void zui_window_show(ZuiWindow *window);
bool zui_window_running(ZuiWindow *window);
void zui_window_render(ZuiWindow *window);

void zui_window_set_title(ZuiWindow *window, const char *title);
void zui_window_set_corner_radius(ZuiWindow *window, float radius);
void zui_window_set_min_size(ZuiWindow *window, int width, int height);
void zui_window_set_max_size(ZuiWindow *window, int width, int height);

ZuiTitlebar *zui_window_titlebar(ZuiWindow *window);
ZuiWidget *zui_window_content(ZuiWindow *window);

void zui_window_minimize(ZuiWindow *window);
void zui_window_maximize(ZuiWindow *window);
void zui_window_close(ZuiWindow *window);
bool zui_window_is_maximized(ZuiWindow *window);

void zui_titlebar_set_start(ZuiTitlebar *titlebar, ZuiWidget *widget);
void zui_titlebar_set_center(ZuiTitlebar *titlebar, ZuiWidget *widget);
void zui_titlebar_set_end(ZuiTitlebar *titlebar, ZuiWidget *widget);

ZuiWidget *zui_window_close_button(ZuiWindow *window);
ZuiWidget *zui_window_minimize_button(ZuiWindow *window);
ZuiWidget *zui_window_maximize_button(ZuiWindow *window);
ZuiWidget *zui_window_hide_button(ZuiWindow *window);

void zui_window_set_logo(ZuiWindow *window, const char *path);
void zui_window_set_logo_size(ZuiWindow *window, float width, float height);
void zui_window_set_logo_visible(ZuiWindow *window, bool visible);
void zui_window_remove_logo(ZuiWindow *window);

void zui_window_set_background_color(ZuiWindow *window, ZuiColor color);
void zui_window_set_border_color(ZuiWindow *window, ZuiColor color);
bool zui_window_is_active(ZuiWindow *window);

#endif
