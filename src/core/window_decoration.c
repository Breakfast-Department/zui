#include <zui/internal/window_internal.h>
#include <zui/internal/widget_internal.h>
#include <zui/image.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

extern ZuiWidget *zui_window_close_button(ZuiWindow *window);
extern ZuiWidget *zui_window_minimize_button(ZuiWindow *window);
extern ZuiWidget *zui_window_maximize_button(ZuiWindow *window);
extern void zui_button_on_click(ZuiButton *button, ZuiClickCallback callback, void *user_data);
extern ZuiLabel *zui_label_create(const char *text);
extern void zui_label_set_text(ZuiLabel *label, const char *text);
extern void zui_label_set_size(ZuiLabel *label, float size);
extern void zui_window_mark_needs_redraw(ZuiWindow *window);
extern ZuiWidget *zui_label_new(const char *text);
extern void zui_window_close(ZuiWindow *window);
extern void zui_window_maximize(ZuiWindow *window);
extern void zui_window_minimize(ZuiWindow *window);
extern ZuiWidget *zui_window_content(ZuiWindow *window);
extern void zui_add_window_moving_handler(ZuiWindow *window, ZuiWidget *widget);
extern void zui_remove_window_moving_handler(ZuiWindow *window, ZuiWidget *widget);
extern void zui_set_window_decoration(ZuiWindow *window, ZuiWidget *decoration);

#define WINDOW_DECORATION_HEIGHT 32.0f
#define WINDOW_DECORATION_PADDING 4.0f

typedef struct ZuiWindowDecoration ZuiWindowDecoration;

struct ZuiWindowDecoration {
  ZuiWidget *container;
  ZuiWidget *start_section;
  ZuiWidget *center_section;
  ZuiWidget *end_section;
  ZuiImage *logo;
  ZuiWidget *title_label;
  ZuiWidget *minimize_btn;
  ZuiWidget *maximize_btn;
  ZuiWidget *close_btn;
  ZuiWindow *window;
};

static void decoration_layout(ZuiWidget *widget);

static const ZuiWidgetVTable decoration_vtable = {
  .layout = decoration_layout,
};

static void section_layout(ZuiWidget *widget)
{
  float x = widget->bounds.x + widget->padding;
  float y = widget->bounds.y + widget->padding;
  float available_w = widget->bounds.width - widget->padding * 2;
  float available_h = widget->bounds.height - widget->padding * 2;

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

    zui_widget_set_bounds(child, cx, cy, child->preferred_size.width, ch);
    cx += child->preferred_size.width + widget->spacing;
  }
}

static const ZuiWidgetVTable section_vtable = {
  .layout = section_layout,
};

static ZuiWidget *create_section(ZuiAlign align)
{
  ZuiWidget *section = zui_widget_create(sizeof(ZuiWidget),
                                          ZUI_WIDGET_CONTAINER,
                                          &section_vtable);
  if (!section) return NULL;

  section->layout_dir = ZUI_LAYOUT_HORIZONTAL;
  section->align = align;
  section->spacing = 8.0f;
  section->padding = WINDOW_DECORATION_PADDING;

  return section;
}

static void decoration_layout(ZuiWidget *widget)
{
  float x = widget->bounds.x;
  float y = widget->bounds.y;
  float w = widget->bounds.width;
  float h = widget->bounds.height;
  float padding = widget->padding;

  float start_w = 0, center_w = 0, end_w = 0;

  ZuiWidget *start = widget->child_count > 0 ? widget->children[0] : NULL;
  ZuiWidget *center = widget->child_count > 1 ? widget->children[1] : NULL;
  ZuiWidget *end = widget->child_count > 2 ? widget->children[2] : NULL;

  if (start) {
    for (int i = 0; i < start->child_count; i++) {
      ZuiWidget *child = start->children[i];
      start_w += child->preferred_size.width;
      if (i > 0) start_w += start->spacing;
    }
    start_w += start->padding * 2;
  }

  if (center) {
    for (int i = 0; i < center->child_count; i++) {
      ZuiWidget *child = center->children[i];
      center_w += child->preferred_size.width;
      if (i > 0) center_w += center->spacing;
    }
    center_w += center->padding * 2;
  }

  if (end) {
    for (int i = 0; i < end->child_count; i++) {
      ZuiWidget *child = end->children[i];
      end_w += child->preferred_size.width;
      if (i > 0) end_w += end->spacing;
    }
    end_w += end->padding * 2;
  }

  float max_side = start_w > end_w ? start_w : end_w;

  if (start) {
    zui_widget_set_bounds(start, x + padding, y, start_w, h);
    zui_widget_layout(start);
  }

  if (center) {
    float center_x = x + (w - center_w) / 2;
    float min_center_x = x + max_side + padding;
    float max_center_x = x + w - max_side - center_w - padding;
    if (center_x < min_center_x) center_x = min_center_x;
    if (center_x > max_center_x) center_x = max_center_x;
    zui_widget_set_bounds(center, center_x, y, center_w, h);
    zui_widget_layout(center);
  }

  if (end) {
    zui_widget_set_bounds(end, x + w - end_w - padding, y, end_w, h);
    zui_widget_layout(end);
  }
}

static void close_click(ZuiWidget *widget, void *user_data)
{
  (void)widget;
  ZuiWindow *window = user_data;
  zui_window_close(window);
}

static void maximize_click(ZuiWidget *widget, void *user_data)
{
  (void)widget;
  ZuiWindow *window = user_data;
  zui_window_maximize(window);
}

static void minimize_click(ZuiWidget *widget, void *user_data)
{
  (void)widget;
  ZuiWindow *window = user_data;
  zui_window_minimize(window);
}

ZuiWindowDecoration *zui_default_window_decoration(ZuiWindow *window)
{
  if (!window) return NULL;

  ZuiWindowDecoration *decor = calloc(1, sizeof(ZuiWindowDecoration));
  if (!decor) return NULL;

  decor->window = window;

  decor->container = zui_widget_create(sizeof(ZuiWidget),
                                        ZUI_WIDGET_CONTAINER,
                                        &decoration_vtable);
  if (!decor->container) {
    free(decor);
    return NULL;
  }

  decor->container->background = ZUI_COLOR(0, 0, 0, 0);
  decor->container->padding = 2.0f;
  decor->container->preferred_size.height = WINDOW_DECORATION_HEIGHT;
  decor->container->fill_width = true;

  decor->start_section = create_section(ZUI_ALIGN_START);
  decor->center_section = create_section(ZUI_ALIGN_CENTER);
  decor->end_section = create_section(ZUI_ALIGN_END);

  if (decor->start_section) {
    zui_widget_add_child(decor->container, decor->start_section);
  }
  if (decor->center_section) {
    zui_widget_add_child(decor->container, decor->center_section);
  }
  if (decor->end_section) {
    zui_widget_add_child(decor->container, decor->end_section);
  }

  decor->minimize_btn = zui_window_minimize_button(window);
  decor->maximize_btn = zui_window_maximize_button(window);
  decor->close_btn = zui_window_close_button(window);

  if (decor->end_section) {
    if (decor->minimize_btn) {
      zui_widget_add_child(decor->end_section, decor->minimize_btn);
      zui_button_on_click((ZuiButton *)decor->minimize_btn, minimize_click, window);
    }
    if (decor->maximize_btn) {
      zui_widget_add_child(decor->end_section, decor->maximize_btn);
      zui_button_on_click((ZuiButton *)decor->maximize_btn, maximize_click, window);
    }
    if (decor->close_btn) {
      zui_widget_add_child(decor->end_section, decor->close_btn);
      zui_button_on_click((ZuiButton *)decor->close_btn, close_click, window);
    }
  }

  zui_add_window_moving_handler(window, decor->container);
  zui_set_window_decoration(window, decor->container);

  ZuiWidget *content = zui_window_content(window);
  if (content && content->child_count == 0) {
    zui_widget_add_child(content, decor->container);
  }

  return decor;
}

void zui_window_decoration_destroy(ZuiWindowDecoration *decor)
{
  if (!decor) return;

  if (decor->window && decor->container) {
    zui_remove_window_moving_handler(decor->window, decor->container);
    zui_set_window_decoration(decor->window, NULL);

    ZuiWidget *content = zui_window_content(decor->window);
    if (content) {
      zui_widget_remove_child(content, decor->container);
    }

    zui_widget_destroy(decor->container);
  }

  if (decor->logo) {
    zui_image_destroy(decor->logo);
  }

  free(decor);
}

void zui_window_decoration_set_title(ZuiWindowDecoration *decor, const char *title)
{
  if (!decor || !decor->center_section) return;

  if (decor->title_label) {
    zui_label_set_text((ZuiLabel *)decor->title_label, title);
  } else {
    decor->title_label = zui_label_new(title);
    if (decor->title_label) {
      zui_label_set_size((ZuiLabel *)decor->title_label, 16.0f);
      zui_widget_add_child(decor->center_section, decor->title_label);
    }
  }

  if (decor->window) {
    zui_window_mark_needs_redraw(decor->window);
  }
}

void zui_window_decoration_set_logo(ZuiWindowDecoration *decor, const char *path)
{
  if (!decor || !decor->start_section) return;

  if (decor->logo) {
    ZuiWidget *logo_widget = zui_image_widget(decor->logo);
    zui_widget_remove_child(decor->start_section, logo_widget);
    zui_image_destroy(decor->logo);
    decor->logo = NULL;
  }

  if (path) {
    decor->logo = zui_image_create(path);
    if (decor->logo) {
      zui_image_set_size(decor->logo, 16.0f, 16.0f);
      ZuiWidget *logo_widget = zui_image_widget(decor->logo);
      if (decor->start_section->child_count > 0) {
        for (int i = decor->start_section->child_count - 1; i >= 0; i--) {
          decor->start_section->children[i + 1] = decor->start_section->children[i];
        }
        decor->start_section->children[0] = logo_widget;
        logo_widget->parent = decor->start_section;
        decor->start_section->child_count++;
      } else {
        zui_widget_add_child(decor->start_section, logo_widget);
      }
    }
  }

  if (decor->window) {
    zui_window_mark_needs_redraw(decor->window);
  }
}

void zui_window_decoration_set_logo_from_memory(ZuiWindowDecoration *decor,
                                                 const unsigned char *data,
                                                 size_t size)
{
  if (!decor || !decor->start_section || !data || size == 0) return;

  if (decor->logo) {
    ZuiWidget *logo_widget = zui_image_widget(decor->logo);
    zui_widget_remove_child(decor->start_section, logo_widget);
    zui_image_destroy(decor->logo);
    decor->logo = NULL;
  }

  decor->logo = zui_image_create_from_memory(data, (int)size);
  if (decor->logo) {
    zui_image_set_size(decor->logo, 16.0f, 16.0f);
    ZuiWidget *logo_widget = zui_image_widget(decor->logo);
    if (decor->start_section->child_count > 0) {
      for (int i = decor->start_section->child_count - 1; i >= 0; i--) {
        decor->start_section->children[i + 1] = decor->start_section->children[i];
      }
      decor->start_section->children[0] = logo_widget;
      logo_widget->parent = decor->start_section;
      decor->start_section->child_count++;
    } else {
      zui_widget_add_child(decor->start_section, logo_widget);
    }
  }

  if (decor->window) {
    zui_window_mark_needs_redraw(decor->window);
  }
}

ZuiWidget *zui_window_decoration_widget(ZuiWindowDecoration *decor)
{
  return decor ? decor->container : NULL;
}
