#ifndef ZUI_WIDGET_H
#define ZUI_WIDGET_H

#include <stdbool.h>
#include <stddef.h>
#include "color.h"
#include "font.h"

typedef struct ZuiWidget ZuiWidget;
typedef struct ZuiButton ZuiButton;
typedef struct ZuiLabel ZuiLabel;

typedef void (*ZuiClickCallback)(ZuiWidget *widget, void *user_data);

typedef enum ZuiCursor {
    ZUI_CURSOR_DEFAULT,
    ZUI_CURSOR_POINTER,
    ZUI_CURSOR_TEXT,
    ZUI_CURSOR_CROSSHAIR,
    ZUI_CURSOR_MOVE,
    ZUI_CURSOR_NOT_ALLOWED,
    ZUI_CURSOR_GRAB,
    ZUI_CURSOR_GRABBING,
} ZuiCursor;

void zui_widget_destroy(ZuiWidget *widget);
void zui_widget_add_child(ZuiWidget *parent, ZuiWidget *child);
void zui_widget_set_visible(ZuiWidget *widget, bool visible);
void zui_widget_set_background(ZuiWidget *widget, ZuiColor color);
void zui_widget_set_corner_radius(ZuiWidget *widget, float radius);
void zui_widget_set_cursor(ZuiWidget *widget, ZuiCursor cursor);

ZuiButton *zui_button_create(const char *text);
void zui_button_set_text(ZuiButton *button, const char *text);
void zui_button_set_size(ZuiButton *button, float width, float height);
void zui_button_set_color(ZuiButton *button, ZuiColor color);
void zui_button_on_click(ZuiButton *button, ZuiClickCallback callback,
                          void *user_data);

ZuiLabel *zui_label_create(const char *text);
void zui_label_set_text(ZuiLabel *label, const char *text);
void zui_label_set_font(ZuiLabel *label, ZuiFont *font);
void zui_label_set_color(ZuiLabel *label, ZuiColor color);

#endif
