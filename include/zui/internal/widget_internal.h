#ifndef ZUI_WIDGET_INTERNAL_H
#define ZUI_WIDGET_INTERNAL_H

#include "types.h"
#include "renderer.h"

#define ZUI_MAX_CHILDREN 64

typedef enum ZuiCornerMode {
  ZUI_CORNERS_ALL,
  ZUI_CORNERS_TOP,
  ZUI_CORNERS_BOTTOM,
  ZUI_CORNERS_NONE,
} ZuiCornerMode;

typedef struct ZuiWidgetVTable {
  void (*draw)(ZuiWidget *widget, ZuiRenderer *renderer);
  void (*draw_overlay)(ZuiWidget *widget, ZuiRenderer *renderer);
  void (*layout)(ZuiWidget *widget);
  void (*destroy)(ZuiWidget *widget);
  bool (*hit_test)(ZuiWidget *widget, float x, float y);
  ZuiWidget *(*hit_test_children)(ZuiWidget *widget, float x, float y);
  void (*on_mouse_enter)(ZuiWidget *widget);
  void (*on_mouse_leave)(ZuiWidget *widget);
  void (*on_mouse_down)(ZuiWidget *widget, float x, float y, uint32_t button);
  void (*on_mouse_up)(ZuiWidget *widget, float x, float y, uint32_t button);
  void (*on_mouse_move)(ZuiWidget *widget, float x, float y);
  void (*on_scroll)(ZuiWidget *widget, double dx, double dy);
  void (*on_key)(ZuiWidget *widget, uint32_t key, uint32_t sym,
                  const char *text, bool pressed);
  void (*on_focus)(ZuiWidget *widget, bool focused);
} ZuiWidgetVTable;

typedef struct ZuiWidget {
  ZuiWidgetType type;
  const ZuiWidgetVTable *vtable;

  ZuiBounds bounds;
  ZuiSize min_size;
  ZuiSize preferred_size;
  bool visible;
  bool hovered;
  bool pressed;

  ZuiWidget *parent;
  ZuiWidget *children[ZUI_MAX_CHILDREN];
  int child_count;

  ZuiColor background;
  float corner_radius;
  ZuiCornerMode corner_mode;
  float padding;
  float spacing;
  ZuiLayoutDir layout_dir;
  ZuiAlign align;
  bool expand;
  bool fill_width;
  bool fill_height;
  bool needs_layout;
  ZuiCursor cursor;

  ZuiClickCallback on_click;
  void *user_data;
} ZuiWidget;

ZuiWidget *zui_widget_create(size_t size, ZuiWidgetType type,
                              const ZuiWidgetVTable *vtable);
void zui_widget_destroy(ZuiWidget *widget);
void zui_widget_add_child(ZuiWidget *parent, ZuiWidget *child);
void zui_widget_remove_child(ZuiWidget *parent, ZuiWidget *child);
void zui_widget_draw(ZuiWidget *widget, ZuiRenderer *renderer);
void zui_widget_layout(ZuiWidget *widget);

bool zui_widget_contains_point(ZuiWidget *widget, float x, float y);
ZuiWidget *zui_widget_hit_test(ZuiWidget *widget, float x, float y);

void zui_widget_set_bounds(ZuiWidget *widget, float x, float y,
                            float width, float height);
void zui_set_visible(ZuiWidget *widget, bool visible);
void zui_set_background(ZuiWidget *widget, ZuiColor color);
void zui_set_corner_radius(ZuiWidget *widget, float radius);
void zui_set_cursor(ZuiWidget *widget, ZuiCursor cursor);
void zui_set_fill(ZuiWidget *widget, bool fill_width, bool fill_height);
void zui_set_size(ZuiWidget *widget, float width, float height);
void zui_set_padding(ZuiWidget *widget, float padding);
void zui_set_spacing(ZuiWidget *widget, float spacing);
void zui_widget_invalidate(ZuiWidget *widget);
bool zui_widget_needs_layout(ZuiWidget *widget);

#endif
