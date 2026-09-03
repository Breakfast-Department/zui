#ifndef ZUI_WIDGET_H
#define ZUI_WIDGET_H

#include <stdbool.h>
#include <stddef.h>
#include "color.h"
#include "font.h"
#include "layout.h"

typedef struct ZuiWidget ZuiWidget;
typedef struct ZuiButton ZuiButton;
typedef struct ZuiLabel ZuiLabel;
typedef struct ZuiPanel ZuiPanel;

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
void zui_widget_set_fill(ZuiWidget *widget, bool fill_width, bool fill_height);

ZuiButton *zui_button_create(const char *text);
void zui_button_set_text(ZuiButton *button, const char *text);
void zui_button_set_size(ZuiButton *button, float width, float height);
void zui_button_set_color(ZuiButton *button, ZuiColor color);
void zui_button_on_click(ZuiButton *button, ZuiClickCallback callback,
                          void *user_data);

ZuiLabel *zui_label_create(const char *text);
void zui_label_set_size(ZuiLabel *label, float size);
void zui_label_set_text(ZuiLabel *label, const char *text);
void zui_label_set_font(ZuiLabel *label, ZuiFont *font);
void zui_label_set_color(ZuiLabel *label, ZuiColor color);

float zui_get_font_size(ZuiLabel *label);

ZuiPanel *zui_panel_create(void);
void zui_panel_destroy(ZuiPanel *panel);
void zui_panel_set_layout(ZuiPanel *panel, ZuiLayoutDir direction);
void zui_panel_set_padding(ZuiPanel *panel, float top, float right, float bottom, float left);
void zui_panel_set_spacing(ZuiPanel *panel, float spacing);
void zui_panel_set_alignment(ZuiPanel *panel, ZuiAlign main_axis, ZuiAlign cross_axis);
void zui_panel_set_background(ZuiPanel *panel, ZuiColor color);
void zui_panel_set_corner_radius(ZuiPanel *panel, float radius);
void zui_panel_set_size(ZuiPanel *panel, float width, float height);
void zui_panel_set_fill(ZuiPanel *panel, bool fill_width, bool fill_height);
void zui_panel_add_child(ZuiPanel *panel, ZuiWidget *child);
void zui_panel_remove_child(ZuiPanel *panel, ZuiWidget *child);
void zui_panel_clear(ZuiPanel *panel);
ZuiWidget *zui_panel_as_widget(ZuiPanel *panel);

#endif
