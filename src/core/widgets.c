#include "zui/color.h"
#include "zui/font.h"
#include "zui/widget.h"
#include <zui/internal/widget_internal.h>
#include <zui/internal/window_internal.h>
#include <zui/internal/icon_internal.h>
#include <zui/internal/font_internal.h>
#include <zui/internal/wayland_platform.h>
#include <zui/resource.h>
#include <zui/app.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <linux/limits.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>

static ZuiIconSource *load_icon_resource(const char *res_path)
{
  size_t size;
  const unsigned char *data = zui_resource_get(res_path, &size);
  if (data && size > 0) {
    return zui_icon_load_svg_data((const char *)data);
  }
  return NULL;
}

extern ZuiPlatform *zui_get_platform(void);

extern void zui_widget_focus(ZuiWidget *widget);

extern ZuiRenderer *zui_get_renderer(void);

static ZuiFont *get_default_font(void);

extern void zui_window_close(ZuiWindow *window);
extern void zui_window_minimize(ZuiWindow *window);
extern void zui_window_maximize(ZuiWindow *window);
extern void zui_window_set_overlay(ZuiWindow *window, ZuiWidget *widget);
extern void zui_window_clear_overlay(ZuiWindow *window, ZuiWidget *widget);
extern void zui_window_mark_needs_redraw(ZuiWindow *window);

struct ZuiButton {
  ZuiWidget base;
  char *text;
  ZuiColor normal_color;
  ZuiColor hover_color;
  ZuiColor pressed_color;
  ZuiColor text_color;
  ZuiFont *font;
  ZuiTexture icon_texture;
  float icon_size;
  float icon_spacing;
  ZuiAlign align;
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

struct ZuiText {
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

  float content_width = 0;
  float text_w = 0, text_h = 0;

  if (button->icon_texture.id) {
    content_width += button->icon_size;
  }
  if (button->text && button->font) {
    text_w = zui_font_text_width(button->font, button->text);
    text_h = zui_font_text_height(button->font, button->text);
    if (button->icon_texture.id) {
      content_width += button->icon_spacing;
    }
    content_width += text_w;
  }

  float content_x = widget->bounds.x + (widget->bounds.width - content_width) / 2;
  float content_y = widget->bounds.y + widget->bounds.height / 2;

  if (button->icon_texture.id) {
    float icon_y = content_y - button->icon_size / 2;
    zui_renderer_draw_texture(renderer, &button->icon_texture,
      ZUI_RECT(content_x, icon_y, button->icon_size, button->icon_size),
      ZUI_COLOR_RGB(1.0f, 1.0f, 1.0f));
    content_x += button->icon_size + button->icon_spacing;
  }

  if (button->text && button->font) {
    float text_y = content_y - text_h / 2;
    zui_font_render_text(button->font, renderer, content_x, text_y,
                          button->text, button->text_color);
  }
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
  if (button->icon_texture.id) {
    zui_texture_destroy(&button->icon_texture);
  }
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
  btn->icon_source = load_icon_resource(svg_path);

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
  button->text_color = ZUI_COLOR_HEX(0xffffff);
  button->font = get_default_font();
  button->icon_texture = (ZuiTexture){0};
  button->icon_size = 16.0f;
  button->icon_spacing = 8.0f;

  button->base.corner_radius = 6.0f;
  button->base.preferred_size.width = 80.0f;
  button->base.preferred_size.height = 32.0f;
  button->base.padding = 8.0f;
  button->base.cursor = ZUI_CURSOR_POINTER;

  return button;
}

void zui_button_set_alignment(ZuiButton *button, ZuiAlign align)
{
  if (!button) return;
  button->align = align;
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

void zui_button_set_icon(ZuiButton *button, const char *image_path, float size)
{
  if (!button || !image_path) return;

  if (button->icon_texture.id) {
    zui_texture_destroy(&button->icon_texture);
  }

  char resolved_path[PATH_MAX];
  if (zui_resolve_asset_path(image_path, resolved_path, sizeof(resolved_path))) {
    button->icon_texture = zui_texture_load(resolved_path);
  } else {
    button->icon_texture = zui_texture_load(image_path);
  }
  button->icon_size = size;
}

void zui_button_set_text_color(ZuiButton *button, ZuiColor color)
{
  if (!button) return;
  button->text_color = color;
}

const char *zui_button_get_text(ZuiButton *button)
{
  return button ? button->text : NULL;
}

ZuiWidget *zui_button_as_widget(ZuiButton *button)
{
  return (ZuiWidget *)button;
}

static void text_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiText *text = (ZuiText *)widget;
  if (!text->text || !text->font) return;

  float text_w = zui_font_text_width(text->font, text->text);
  float text_h = zui_font_text_height(text->font, text->text);

  float x = widget->bounds.x + (widget->bounds.width - text_w) / 2;
  float y = widget->bounds.y + (widget->bounds.height - text_h) / 2;

  zui_font_render_text(text->font, renderer, x, y, text->text, text->text_color);
}

static void text_destroy(ZuiWidget *widget)
{
  ZuiText *text = (ZuiText *)widget;
  free(text->text);
  if (text->owns_font && text->font) {
    zui_font_destroy(text->font);
  }
}

static const ZuiWidgetVTable text_vtable = {
  .draw = text_draw,
  .destroy = text_destroy,
};

struct ZuiCheckbox {
  ZuiWidget base;
  char *text;
  bool checked;
  ZuiColor box_color;
  ZuiColor checked_box_color;
  ZuiColor icon_color;
  ZuiColor text_color;
  ZuiFont *font;
  float box_size;
  float corner_radius;
  ZuiIconSource *check_icon;
  ZuiTexture check_texture;
  ZuiCheckboxCallback on_change;
  void *change_user_data;
};

static void checkbox_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiCheckbox *cb = (ZuiCheckbox *)widget;

  float box_x = widget->bounds.x;
  float box_y = widget->bounds.y + (widget->bounds.height - cb->box_size) / 2;

  ZuiColor box_color = cb->checked ? cb->checked_box_color : cb->box_color;
  if (widget->hovered) {
    box_color = ZUI_COLOR(box_color.r * 1.2f, box_color.g * 1.2f,
                           box_color.b * 1.2f, box_color.a);
  }

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(box_x, box_y, cb->box_size, cb->box_size),
    box_color, cb->corner_radius);

  if (cb->checked && cb->check_texture.id) {
    float icon_size = cb->box_size * 0.85f;
    float icon_x = box_x + (cb->box_size - icon_size) / 2.0f;
    float icon_y = box_y + (cb->box_size - icon_size) / 2.0f;
    zui_renderer_draw_texture(renderer, &cb->check_texture,
      ZUI_RECT(icon_x, icon_y, icon_size, icon_size),
      cb->icon_color);
  }

  if (cb->text && cb->font) {
    float text_x = box_x + cb->box_size + 8.0f;
    float text_h = zui_font_text_height(cb->font, cb->text);
    float text_y = widget->bounds.y + (widget->bounds.height - text_h) / 2;
    zui_font_render_text(cb->font, renderer, text_x, text_y,
                          cb->text, cb->text_color);
  }
}

static bool checkbox_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)x;
  (void)y;
  return true;
}

static void checkbox_on_mouse_enter(ZuiWidget *widget)
{
  (void)widget;
}

static void checkbox_on_mouse_leave(ZuiWidget *widget)
{
  (void)widget;
}

static void checkbox_on_mouse_up(ZuiWidget *widget, float x, float y,
                                  uint32_t button)
{
  (void)x;
  (void)y;
  if (button != BTN_LEFT) return;

  ZuiCheckbox *cb = (ZuiCheckbox *)widget;
  cb->checked = !cb->checked;

  if (cb->on_change) {
    cb->on_change(cb, cb->checked, cb->change_user_data);
  }
}

static void checkbox_destroy(ZuiWidget *widget)
{
  ZuiCheckbox *cb = (ZuiCheckbox *)widget;
  free(cb->text);
  if (cb->check_texture.id) {
    zui_texture_destroy(&cb->check_texture);
  }
  if (cb->check_icon) {
    zui_icon_source_destroy(cb->check_icon);
  }
}

static const ZuiWidgetVTable checkbox_vtable = {
  .draw = checkbox_draw,
  .hit_test = checkbox_hit_test,
  .on_mouse_enter = checkbox_on_mouse_enter,
  .on_mouse_leave = checkbox_on_mouse_leave,
  .on_mouse_up = checkbox_on_mouse_up,
  .destroy = checkbox_destroy,
};

static ZuiFont *get_default_font(void);

ZuiCheckbox *zui_checkbox_create(const char *text)
{
  ZuiCheckbox *cb = (ZuiCheckbox *)zui_widget_create(
    sizeof(ZuiCheckbox), ZUI_WIDGET_CHECKBOX, &checkbox_vtable);
  if (!cb) return NULL;

  cb->text = text ? strdup(text) : NULL;
  cb->checked = false;
  cb->box_color = ZUI_COLOR_HEX(0x4d4d4d);
  cb->checked_box_color = ZUI_COLOR_HEX(0x4a9eff);
  cb->icon_color = ZUI_COLOR_RGB(1.0f, 1.0f, 1.0f);
  cb->text_color = ZUI_COLOR_HEX(0xffffff);
  cb->font = get_default_font();
  cb->box_size = 18.0f;
  cb->corner_radius = 4.0f;
  cb->on_change = NULL;
  cb->change_user_data = NULL;

  cb->check_icon = load_icon_resource("res:/zui/icons/check.svg");
  if (cb->check_icon) {
    cb->check_texture = zui_texture_create(cb->check_icon->raster_data,
                                            cb->check_icon->raster_width,
                                            cb->check_icon->raster_height);
  }

  cb->base.cursor = ZUI_CURSOR_POINTER;

  float text_width = 0;
  if (cb->font && text) {
    text_width = zui_font_text_width(cb->font, text);
  }
  cb->base.preferred_size.width = cb->box_size + 8.0f + text_width;
  cb->base.preferred_size.height = cb->box_size + 8.0f;

  return cb;
}

void zui_checkbox_set_checked(ZuiCheckbox *checkbox, bool checked)
{
  if (!checkbox) return;
  checkbox->checked = checked;
}

bool zui_checkbox_is_checked(ZuiCheckbox *checkbox)
{
  if (!checkbox) return false;
  return checkbox->checked;
}

void zui_checkbox_set_text(ZuiCheckbox *checkbox, const char *text)
{
  if (!checkbox) return;
  free(checkbox->text);
  checkbox->text = text ? strdup(text) : NULL;

  float text_width = 0;
  if (checkbox->font && text) {
    text_width = zui_font_text_width(checkbox->font, text);
  }
  checkbox->base.preferred_size.width = checkbox->box_size + 8.0f + text_width;
}

void zui_checkbox_set_size(ZuiCheckbox *checkbox, float size)
{
  if (!checkbox) return;
  checkbox->box_size = size;

  float text_width = 0;
  if (checkbox->font && checkbox->text) {
    text_width = zui_font_text_width(checkbox->font, checkbox->text);
  }
  checkbox->base.preferred_size.width = size + 8.0f + text_width;
  checkbox->base.preferred_size.height = size + 8.0f;
}

void zui_checkbox_on_change(ZuiCheckbox *checkbox, ZuiCheckboxCallback callback,
                             void *user_data)
{
  if (!checkbox) return;
  checkbox->on_change = callback;
  checkbox->change_user_data = user_data;
}

void zui_checkbox_set_box_color(ZuiCheckbox *checkbox, ZuiColor unchecked,
                                 ZuiColor checked)
{
  if (!checkbox) return;
  checkbox->box_color = unchecked;
  checkbox->checked_box_color = checked;
}

void zui_checkbox_set_icon_color(ZuiCheckbox *checkbox, ZuiColor color)
{
  if (!checkbox) return;
  checkbox->icon_color = color;
}

void zui_checkbox_set_icon(ZuiCheckbox *checkbox, const char *svg_path)
{
  if (!checkbox || !svg_path) return;

  if (checkbox->check_texture.id) {
    zui_texture_destroy(&checkbox->check_texture);
  }
  if (checkbox->check_icon) {
    zui_icon_source_destroy(checkbox->check_icon);
  }

  checkbox->check_icon = load_icon_resource(svg_path);
  if (checkbox->check_icon) {
    checkbox->check_texture = zui_texture_create(
      checkbox->check_icon->raster_data,
      checkbox->check_icon->raster_width,
      checkbox->check_icon->raster_height);
  }
}

ZuiWidget *zui_checkbox_as_widget(ZuiCheckbox *checkbox)
{
  return (ZuiWidget *)checkbox;
}

struct ZuiRadioGroup {
  ZuiRadioButton **buttons;
  size_t count;
  size_t capacity;
  ZuiRadioButton *selected;
  ZuiRadioCallback on_change;
  void *change_user_data;
};

struct ZuiRadioButton {
  ZuiWidget base;
  char *text;
  ZuiRadioGroup *group;
  ZuiColor circle_color;
  ZuiColor selected_color;
  ZuiColor dot_color;
  ZuiColor text_color;
  ZuiFont *font;
  float size;
};

static void radiobutton_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiRadioButton *rb = (ZuiRadioButton *)widget;

  float cx = widget->bounds.x + rb->size / 2.0f;
  float cy = widget->bounds.y + widget->bounds.height / 2.0f;
  float radius = rb->size / 2.0f;

  bool selected = rb->group && rb->group->selected == rb;

  ZuiColor circle_color = selected ? rb->selected_color : rb->circle_color;
  if (widget->hovered) {
    circle_color = ZUI_COLOR(circle_color.r * 1.2f, circle_color.g * 1.2f,
                              circle_color.b * 1.2f, circle_color.a);
  }

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(cx - radius, cy - radius, rb->size, rb->size),
    circle_color, radius);

  if (selected) {
    float dot_radius = radius * 0.5f;
    zui_renderer_draw_rounded_rect(renderer,
      ZUI_RECT(cx - dot_radius, cy - dot_radius, dot_radius * 2, dot_radius * 2),
      rb->dot_color, dot_radius);
  }

  if (rb->text && rb->font) {
    float text_x = widget->bounds.x + rb->size + 8.0f;
    float text_h = zui_font_text_height(rb->font, rb->text);
    float text_y = widget->bounds.y + (widget->bounds.height - text_h) / 2;
    zui_font_render_text(rb->font, renderer, text_x, text_y,
                          rb->text, rb->text_color);
  }
}

static bool radiobutton_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static void radiobutton_on_mouse_up(ZuiWidget *widget, float x, float y,
                                     uint32_t button)
{
  (void)x;
  (void)y;
  if (button != BTN_LEFT) return;

  ZuiRadioButton *rb = (ZuiRadioButton *)widget;
  if (!rb->group) return;

  if (rb->group->selected != rb) {
    rb->group->selected = rb;
    if (rb->group->on_change) {
      rb->group->on_change(rb->group, rb, rb->group->change_user_data);
    }
  }
}

static void radiobutton_destroy(ZuiWidget *widget)
{
  ZuiRadioButton *rb = (ZuiRadioButton *)widget;
  free(rb->text);
}

static const ZuiWidgetVTable radiobutton_vtable = {
  .draw = radiobutton_draw,
  .hit_test = radiobutton_hit_test,
  .on_mouse_up = radiobutton_on_mouse_up,
  .destroy = radiobutton_destroy,
};

ZuiRadioGroup *zui_radiogroup_create(void)
{
  ZuiRadioGroup *group = calloc(1, sizeof(ZuiRadioGroup));
  if (!group) return NULL;

  group->buttons = NULL;
  group->count = 0;
  group->capacity = 0;
  group->selected = NULL;
  group->on_change = NULL;
  group->change_user_data = NULL;

  return group;
}

void zui_radiogroup_destroy(ZuiRadioGroup *group)
{
  if (!group) return;
  free(group->buttons);
  free(group);
}

void zui_radiogroup_on_change(ZuiRadioGroup *group, ZuiRadioCallback callback,
                               void *user_data)
{
  if (!group) return;
  group->on_change = callback;
  group->change_user_data = user_data;
}

ZuiRadioButton *zui_radiogroup_get_selected(ZuiRadioGroup *group)
{
  if (!group) return NULL;
  return group->selected;
}

int zui_radiogroup_get_selected_index(ZuiRadioGroup *group)
{
  if (!group || !group->selected) return -1;
  for (size_t i = 0; i < group->count; i++) {
    if (group->buttons[i] == group->selected) {
      return (int)i;
    }
  }
  return -1;
}

void zui_radiogroup_select(ZuiRadioGroup *group, size_t index)
{
  if (!group || index >= group->count) return;
  ZuiRadioButton *rb = group->buttons[index];
  if (group->selected != rb) {
    group->selected = rb;
    if (group->on_change) {
      group->on_change(group, rb, group->change_user_data);
    }
  }
}

ZuiRadioButton *zui_radiobutton_create(ZuiRadioGroup *group, const char *text)
{
  if (!group) return NULL;

  ZuiRadioButton *rb = (ZuiRadioButton *)zui_widget_create(
    sizeof(ZuiRadioButton), ZUI_WIDGET_RADIOBUTTON, &radiobutton_vtable);
  if (!rb) return NULL;

  rb->text = text ? strdup(text) : NULL;
  rb->group = group;
  rb->circle_color = ZUI_COLOR_HEX(0x4d4d4d);
  rb->selected_color = ZUI_COLOR_HEX(0x4a9eff);
  rb->dot_color = ZUI_COLOR_RGB(1.0f, 1.0f, 1.0f);
  rb->text_color = ZUI_COLOR_HEX(0xffffff);
  rb->font = get_default_font();
  rb->size = 18.0f;

  rb->base.cursor = ZUI_CURSOR_POINTER;

  float text_width = 0;
  if (rb->font && text) {
    text_width = zui_font_text_width(rb->font, text);
  }
  rb->base.preferred_size.width = rb->size + 8.0f + text_width;
  rb->base.preferred_size.height = rb->size + 8.0f;

  if (group->count >= group->capacity) {
    size_t new_cap = group->capacity == 0 ? 4 : group->capacity * 2;
    ZuiRadioButton **new_buttons = realloc(group->buttons,
                                            new_cap * sizeof(ZuiRadioButton *));
    if (!new_buttons) {
      zui_widget_destroy((ZuiWidget *)rb);
      return NULL;
    }
    group->buttons = new_buttons;
    group->capacity = new_cap;
  }

  group->buttons[group->count] = rb;
  group->count++;

  if (group->count == 1) {
    group->selected = rb;
  }

  return rb;
}

void zui_radiobutton_set_colors(ZuiRadioButton *rb, ZuiColor circle,
                                 ZuiColor selected, ZuiColor dot)
{
  if (!rb) return;
  rb->circle_color = circle;
  rb->selected_color = selected;
  rb->dot_color = dot;
}

void zui_radiobutton_set_text_color(ZuiRadioButton *rb, ZuiColor color)
{
  if (!rb) return;
  rb->text_color = color;
}

void zui_radiobutton_set_size(ZuiRadioButton *rb, float size)
{
  if (!rb) return;
  rb->size = size;

  float text_width = 0;
  if (rb->font && rb->text) {
    text_width = zui_font_text_width(rb->font, rb->text);
  }
  rb->base.preferred_size.width = size + 8.0f + text_width;
  rb->base.preferred_size.height = size + 8.0f;
}

const char *zui_radiobutton_get_text(ZuiRadioButton *rb)
{
  if (!rb) return NULL;
  return rb->text;
}

ZuiWidget *zui_radiobutton_as_widget(ZuiRadioButton *rb)
{
  return (ZuiWidget *)rb;
}

#define ZUI_TEXTINPUT_MAX_LEN 1024

struct ZuiTextInput {
  ZuiWidget base;
  char text[ZUI_TEXTINPUT_MAX_LEN];
  char *placeholder;
  size_t text_len;
  size_t cursor_pos;
  size_t selection_start;
  size_t selection_end;
  bool focused;
  bool dragging;
  ZuiFont *font;
  ZuiColor bg_color;
  ZuiColor text_color;
  ZuiColor placeholder_color;
  ZuiColor cursor_color;
  ZuiColor border_color;
  ZuiColor selection_color;
  float corner_radius;
  float padding;
  float scroll_offset;
  ZuiTextInputCallback on_change;
  void *change_user_data;
};

static void textinput_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiTextInput *ti = (ZuiTextInput *)widget;

  ZuiColor border = ti->border_color;
  if (ti->focused) {
    border = ZUI_COLOR(border.r * 1.5f, border.g * 1.5f, border.b * 1.5f, border.a);
  }

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x, widget->bounds.y,
             widget->bounds.width, widget->bounds.height),
    border, ti->corner_radius);

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x + 1, widget->bounds.y + 1,
             widget->bounds.width - 2, widget->bounds.height - 2),
    ti->bg_color, ti->corner_radius - 1);

  float text_x = widget->bounds.x + ti->padding - ti->scroll_offset;
  float text_h = ti->font ? zui_font_text_height(ti->font, "Ay") : 14.0f;
  float text_y = widget->bounds.y + (widget->bounds.height - text_h) / 2;

  zui_renderer_push_clip(renderer,
    ZUI_RECT(widget->bounds.x + ti->padding, widget->bounds.y,
             widget->bounds.width - ti->padding * 2, widget->bounds.height),
    ti->corner_radius);

  if (ti->focused && ti->font && ti->selection_start != ti->selection_end) {
    size_t sel_min = ti->selection_start < ti->selection_end ?
                     ti->selection_start : ti->selection_end;
    size_t sel_max = ti->selection_start > ti->selection_end ?
                     ti->selection_start : ti->selection_end;

    char tmp[ZUI_TEXTINPUT_MAX_LEN];
    memcpy(tmp, ti->text, sel_min);
    tmp[sel_min] = '\0';
    float sel_start_x = text_x + zui_font_text_width(ti->font, tmp);

    memcpy(tmp, ti->text, sel_max);
    tmp[sel_max] = '\0';
    float sel_end_x = text_x + zui_font_text_width(ti->font, tmp);

    zui_renderer_draw_rect(renderer,
      ZUI_RECT(sel_start_x, text_y, sel_end_x - sel_start_x, text_h),
      ti->selection_color);
  }

  if (ti->text_len > 0 && ti->font) {
    zui_font_render_text(ti->font, renderer, text_x, text_y,
                          ti->text, ti->text_color);
  } else if (ti->placeholder && ti->font) {
    zui_font_render_text(ti->font, renderer, text_x, text_y,
                          ti->placeholder, ti->placeholder_color);
  }

  if (ti->focused && ti->font && ti->selection_start == ti->selection_end) {
    char tmp[ZUI_TEXTINPUT_MAX_LEN];
    size_t len = ti->cursor_pos < ti->text_len ? ti->cursor_pos : ti->text_len;
    memcpy(tmp, ti->text, len);
    tmp[len] = '\0';

    float cursor_x = text_x + zui_font_text_width(ti->font, tmp);
    float cursor_h = text_h;
    float cursor_y = text_y;

    zui_renderer_draw_rect(renderer,
      ZUI_RECT(cursor_x, cursor_y, 2.0f, cursor_h),
      ti->cursor_color);
  }

  zui_renderer_pop_clip(renderer);
}

static bool textinput_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static size_t textinput_pos_from_x(ZuiTextInput *ti, float x);
static void textinput_update_scroll(ZuiTextInput *ti);

static void textinput_on_mouse_down(ZuiWidget *widget, float x, float y,
                                     uint32_t button)
{
  (void)y;
  if (button != BTN_LEFT) return;

  ZuiTextInput *ti = (ZuiTextInput *)widget;
  zui_widget_focus(widget);

  size_t pos = textinput_pos_from_x(ti, x);
  ti->cursor_pos = pos;
  ti->selection_start = ti->selection_end = pos;
  ti->dragging = true;
}

static void textinput_on_mouse_up(ZuiWidget *widget, float x, float y,
                                   uint32_t button)
{
  (void)x;
  (void)y;
  if (button != BTN_LEFT) return;

  ZuiTextInput *ti = (ZuiTextInput *)widget;
  ti->dragging = false;
}

static void textinput_on_mouse_move(ZuiWidget *widget, float x, float y)
{
  (void)y;
  ZuiTextInput *ti = (ZuiTextInput *)widget;
  if (!ti->dragging) return;

  size_t pos = textinput_pos_from_x(ti, x);
  ti->cursor_pos = pos;
  ti->selection_end = pos;
  textinput_update_scroll(ti);
}

static void textinput_on_focus(ZuiWidget *widget, bool focused)
{
  ZuiTextInput *ti = (ZuiTextInput *)widget;
  ti->focused = focused;
}

static size_t utf8_prev_char(const char *str, size_t pos)
{
  if (pos == 0) return 0;
  pos--;
  while (pos > 0 && (str[pos] & 0xC0) == 0x80) {
    pos--;
  }
  return pos;
}

static size_t utf8_next_char(const char *str, size_t pos, size_t len)
{
  if (pos >= len) return len;
  pos++;
  while (pos < len && (str[pos] & 0xC0) == 0x80) {
    pos++;
  }
  return pos;
}

static bool is_word_char(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '_';
}

static size_t word_prev(const char *str, size_t pos)
{
  if (pos == 0) return 0;
  pos--;
  while (pos > 0 && !is_word_char(str[pos])) pos--;
  while (pos > 0 && is_word_char(str[pos - 1])) pos--;
  return pos;
}

static size_t word_next(const char *str, size_t pos, size_t len)
{
  if (pos >= len) return len;
  while (pos < len && !is_word_char(str[pos])) pos++;
  while (pos < len && is_word_char(str[pos])) pos++;
  return pos;
}

static void collect_textinputs(ZuiWidget *widget, ZuiWidget **list,
                                size_t *count, size_t max)
{
  if (!widget || !widget->visible) return;

  if (widget->type == ZUI_WIDGET_TEXTINPUT && *count < max) {
    list[(*count)++] = widget;
  }

  for (int i = 0; i < widget->child_count; i++) {
    collect_textinputs(widget->children[i], list, count, max);
  }
}

static ZuiWidget *find_parent_window_widget(ZuiWidget *widget)
{
  while (widget) {
    if (widget->type == ZUI_WIDGET_WINDOW) return widget;
    widget = widget->parent;
  }
  return NULL;
}

static ZuiWidget *find_adjacent_textinput(ZuiWidget *current, bool next)
{
  ZuiWidget *window = find_parent_window_widget(current);
  if (!window) return NULL;

  ZuiWidget *list[64];
  size_t count = 0;
  collect_textinputs(window, list, &count, 64);

  if (count < 2) return NULL;

  for (size_t i = 0; i < count; i++) {
    if (list[i] == current) {
      if (next && i + 1 < count) return list[i + 1];
      if (!next && i > 0) return list[i - 1];
      return NULL;
    }
  }
  return NULL;
}

static size_t textinput_pos_from_x(ZuiTextInput *ti, float x)
{
  if (!ti->font || ti->text_len == 0) return 0;

  float text_x = ti->base.bounds.x + ti->padding - ti->scroll_offset;
  float click_x = x - text_x;

  size_t pos = 0;
  char tmp[ZUI_TEXTINPUT_MAX_LEN];

  for (size_t i = 0; i <= ti->text_len; i++) {
    memcpy(tmp, ti->text, i);
    tmp[i] = '\0';
    float w = zui_font_text_width(ti->font, tmp);
    if (w >= click_x) {
      pos = i > 0 ? i - 1 : 0;
      float prev_w = 0;
      if (i > 0) {
        tmp[i - 1] = '\0';
        prev_w = zui_font_text_width(ti->font, tmp);
      }
      if (click_x - prev_w > w - click_x) {
        pos = i;
      }
      return pos;
    }
    pos = i;
  }
  return pos;
}

static void textinput_update_scroll(ZuiTextInput *ti)
{
  if (!ti->font) return;

  char tmp[ZUI_TEXTINPUT_MAX_LEN];
  size_t len = ti->cursor_pos < ti->text_len ? ti->cursor_pos : ti->text_len;
  memcpy(tmp, ti->text, len);
  tmp[len] = '\0';

  float cursor_x = zui_font_text_width(ti->font, tmp);
  float visible_width = ti->base.bounds.width - ti->padding * 2;

  if (cursor_x - ti->scroll_offset > visible_width) {
    ti->scroll_offset = cursor_x - visible_width + 10;
  } else if (cursor_x < ti->scroll_offset) {
    ti->scroll_offset = cursor_x > 10 ? cursor_x - 10 : 0;
  }
}

static bool textinput_has_selection(ZuiTextInput *ti)
{
  return ti->selection_start != ti->selection_end;
}

static void textinput_delete_selection(ZuiTextInput *ti)
{
  if (!textinput_has_selection(ti)) return;

  size_t sel_min = ti->selection_start < ti->selection_end ?
                   ti->selection_start : ti->selection_end;
  size_t sel_max = ti->selection_start > ti->selection_end ?
                   ti->selection_start : ti->selection_end;

  memmove(ti->text + sel_min, ti->text + sel_max,
          ti->text_len - sel_max + 1);
  ti->text_len -= (sel_max - sel_min);
  ti->cursor_pos = sel_min;
  ti->selection_start = ti->selection_end = ti->cursor_pos;
}

static void textinput_clear_selection(ZuiTextInput *ti)
{
  ti->selection_start = ti->selection_end = ti->cursor_pos;
}

static void textinput_on_key(ZuiWidget *widget, uint32_t key, uint32_t sym,
                              const char *text, bool pressed)
{
  (void)key;
  if (!pressed) return;

  ZuiTextInput *ti = (ZuiTextInput *)widget;
  ZuiPlatform *platform = zui_get_platform();
  bool ctrl = zui_platform_get_ctrl(platform);
  bool shift = zui_platform_get_shift(platform);

  if (sym == XKB_KEY_a && ctrl) {
    ti->selection_start = 0;
    ti->selection_end = ti->text_len;
    ti->cursor_pos = ti->text_len;
    textinput_update_scroll(ti);
    return;
  }

  if (sym == XKB_KEY_BackSpace) {
    if (textinput_has_selection(ti)) {
      textinput_delete_selection(ti);
      if (ti->on_change) {
        ti->on_change(ti, ti->text, ti->change_user_data);
      }
    } else if (ti->cursor_pos > 0) {
      size_t prev;
      if (ctrl) {
        prev = word_prev(ti->text, ti->cursor_pos);
      } else {
        prev = utf8_prev_char(ti->text, ti->cursor_pos);
      }
      size_t del_len = ti->cursor_pos - prev;
      memmove(ti->text + prev, ti->text + ti->cursor_pos,
              ti->text_len - ti->cursor_pos + 1);
      ti->text_len -= del_len;
      ti->cursor_pos = prev;
      textinput_clear_selection(ti);

      if (ti->on_change) {
        ti->on_change(ti, ti->text, ti->change_user_data);
      }
    }
  } else if (sym == XKB_KEY_Delete) {
    if (textinput_has_selection(ti)) {
      textinput_delete_selection(ti);
      if (ti->on_change) {
        ti->on_change(ti, ti->text, ti->change_user_data);
      }
    } else if (ti->cursor_pos < ti->text_len) {
      size_t next;
      if (ctrl) {
        next = word_next(ti->text, ti->cursor_pos, ti->text_len);
      } else {
        next = utf8_next_char(ti->text, ti->cursor_pos, ti->text_len);
      }
      size_t del_len = next - ti->cursor_pos;
      memmove(ti->text + ti->cursor_pos, ti->text + next,
              ti->text_len - next + 1);
      ti->text_len -= del_len;
      textinput_clear_selection(ti);

      if (ti->on_change) {
        ti->on_change(ti, ti->text, ti->change_user_data);
      }
    }
  } else if (sym == XKB_KEY_Left) {
    size_t new_pos;
    if (ctrl) {
      new_pos = word_prev(ti->text, ti->cursor_pos);
    } else {
      new_pos = utf8_prev_char(ti->text, ti->cursor_pos);
    }

    if (shift) {
      if (ti->selection_start == ti->selection_end) {
        ti->selection_start = ti->cursor_pos;
      }
      ti->selection_end = new_pos;
    } else {
      textinput_clear_selection(ti);
    }
    ti->cursor_pos = new_pos;
  } else if (sym == XKB_KEY_Right) {
    size_t new_pos;
    if (ctrl) {
      new_pos = word_next(ti->text, ti->cursor_pos, ti->text_len);
    } else {
      new_pos = utf8_next_char(ti->text, ti->cursor_pos, ti->text_len);
    }

    if (shift) {
      if (ti->selection_start == ti->selection_end) {
        ti->selection_start = ti->cursor_pos;
      }
      ti->selection_end = new_pos;
    } else {
      textinput_clear_selection(ti);
    }
    ti->cursor_pos = new_pos;
  } else if (sym == XKB_KEY_Up) {
    if (ctrl) {
      ZuiWidget *prev = find_adjacent_textinput(widget, false);
      if (prev) {
        zui_widget_focus(prev);
        return;
      }
    }
    if (shift) {
      if (ti->selection_start == ti->selection_end) {
        ti->selection_start = ti->cursor_pos;
      }
      ti->selection_end = 0;
    } else {
      textinput_clear_selection(ti);
    }
    ti->cursor_pos = 0;
  } else if (sym == XKB_KEY_Down) {
    if (ctrl) {
      ZuiWidget *next = find_adjacent_textinput(widget, true);
      if (next) {
        zui_widget_focus(next);
        return;
      }
    }
    if (shift) {
      if (ti->selection_start == ti->selection_end) {
        ti->selection_start = ti->cursor_pos;
      }
      ti->selection_end = ti->text_len;
    } else {
      textinput_clear_selection(ti);
    }
    ti->cursor_pos = ti->text_len;
  } else if (sym == XKB_KEY_Home) {
    if (shift) {
      if (ti->selection_start == ti->selection_end) {
        ti->selection_start = ti->cursor_pos;
      }
      ti->selection_end = 0;
    } else {
      textinput_clear_selection(ti);
    }
    ti->cursor_pos = 0;
  } else if (sym == XKB_KEY_End) {
    if (shift) {
      if (ti->selection_start == ti->selection_end) {
        ti->selection_start = ti->cursor_pos;
      }
      ti->selection_end = ti->text_len;
    } else {
      textinput_clear_selection(ti);
    }
    ti->cursor_pos = ti->text_len;
  } else if (text && text[0] && text[0] >= 0x20 && !ctrl) {
    if (textinput_has_selection(ti)) {
      textinput_delete_selection(ti);
    }

    size_t add_len = strlen(text);
    if (ti->text_len + add_len < ZUI_TEXTINPUT_MAX_LEN - 1) {
      memmove(ti->text + ti->cursor_pos + add_len,
              ti->text + ti->cursor_pos,
              ti->text_len - ti->cursor_pos + 1);
      memcpy(ti->text + ti->cursor_pos, text, add_len);
      ti->text_len += add_len;
      ti->cursor_pos += add_len;
      textinput_clear_selection(ti);

      if (ti->on_change) {
        ti->on_change(ti, ti->text, ti->change_user_data);
      }
    }
  }

  textinput_update_scroll(ti);
}

static void textinput_destroy(ZuiWidget *widget)
{
  ZuiTextInput *ti = (ZuiTextInput *)widget;
  free(ti->placeholder);
}

static const ZuiWidgetVTable textinput_vtable = {
  .draw = textinput_draw,
  .hit_test = textinput_hit_test,
  .on_mouse_down = textinput_on_mouse_down,
  .on_mouse_up = textinput_on_mouse_up,
  .on_mouse_move = textinput_on_mouse_move,
  .on_focus = textinput_on_focus,
  .on_key = textinput_on_key,
  .destroy = textinput_destroy,
};

ZuiTextInput *zui_textinput_create(const char *placeholder)
{
  ZuiTextInput *ti = (ZuiTextInput *)zui_widget_create(
    sizeof(ZuiTextInput), ZUI_WIDGET_TEXTINPUT, &textinput_vtable);
  if (!ti) return NULL;

  ti->text[0] = '\0';
  ti->placeholder = placeholder ? strdup(placeholder) : NULL;
  ti->text_len = 0;
  ti->cursor_pos = 0;
  ti->selection_start = 0;
  ti->selection_end = 0;
  ti->focused = false;
  ti->dragging = false;
  ti->font = get_default_font();
  ti->bg_color = ZUI_COLOR_HEX(0x1a1a1a);
  ti->text_color = ZUI_COLOR_HEX(0xffffff);
  ti->placeholder_color = ZUI_COLOR_HEX(0x666666);
  ti->cursor_color = ZUI_COLOR_HEX(0x4a9eff);
  ti->border_color = ZUI_COLOR_HEX(0x444444);
  ti->selection_color = ZUI_COLOR(0.29f, 0.62f, 1.0f, 0.4f);
  ti->corner_radius = 6.0f;
  ti->padding = 10.0f;
  ti->scroll_offset = 0.0f;
  ti->on_change = NULL;
  ti->change_user_data = NULL;

  ti->base.cursor = ZUI_CURSOR_TEXT;
  ti->base.preferred_size.width = 200.0f;
  ti->base.preferred_size.height = 36.0f;

  return ti;
}

void zui_textinput_set_text(ZuiTextInput *input, const char *text)
{
  if (!input) return;
  if (text) {
    size_t len = strlen(text);
    if (len >= ZUI_TEXTINPUT_MAX_LEN) len = ZUI_TEXTINPUT_MAX_LEN - 1;
    memcpy(input->text, text, len);
    input->text[len] = '\0';
    input->text_len = len;
    input->cursor_pos = len;
  } else {
    input->text[0] = '\0';
    input->text_len = 0;
    input->cursor_pos = 0;
  }
  input->scroll_offset = 0;
}

const char *zui_textinput_get_text(ZuiTextInput *input)
{
  if (!input) return "";
  return input->text;
}

void zui_textinput_set_placeholder(ZuiTextInput *input, const char *placeholder)
{
  if (!input) return;
  free(input->placeholder);
  input->placeholder = placeholder ? strdup(placeholder) : NULL;
}

void zui_textinput_set_size(ZuiTextInput *input, float width, float height)
{
  if (!input) return;
  input->base.preferred_size.width = width;
  input->base.preferred_size.height = height;
}

void zui_textinput_set_colors(ZuiTextInput *input, ZuiColor background,
                               ZuiColor text, ZuiColor placeholder,
                               ZuiColor cursor, ZuiColor border)
{
  if (!input) return;
  input->bg_color = background;
  input->text_color = text;
  input->placeholder_color = placeholder;
  input->cursor_color = cursor;
  input->border_color = border;
}

void zui_textinput_on_change(ZuiTextInput *input, ZuiTextInputCallback callback,
                              void *user_data)
{
  if (!input) return;
  input->on_change = callback;
  input->change_user_data = user_data;
}

ZuiWidget *zui_textinput_as_widget(ZuiTextInput *input)
{
  return (ZuiWidget *)input;
}

typedef void (*ZuiScrollViewCallback)(ZuiScrollView *sv, float x, float y,
                                       void *user_data);

struct ZuiScrollView {
  ZuiWidget base;
  ZuiWidget *content;
  float scroll_x;
  float scroll_y;
  float content_width;
  float content_height;
  ZuiScrollViewCallback on_scroll_cb;
  void *scroll_user_data;
};

static void scrollview_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiScrollView *sv = (ZuiScrollView *)widget;

  if (widget->background.a > 0) {
    zui_renderer_draw_rounded_rect(renderer,
      ZUI_RECT(widget->bounds.x, widget->bounds.y,
               widget->bounds.width, widget->bounds.height),
      widget->background, widget->corner_radius);
  }

  if (!sv->content || !sv->content->visible) return;

  zui_renderer_push_clip(renderer,
    ZUI_RECT(widget->bounds.x, widget->bounds.y,
             widget->bounds.width, widget->bounds.height),
    widget->corner_radius);

  zui_widget_draw(sv->content, renderer);

  zui_renderer_pop_clip(renderer);
}

static void scrollview_layout(ZuiWidget *widget)
{
  ZuiScrollView *sv = (ZuiScrollView *)widget;
  if (!sv->content) return;

  float content_w = sv->content->preferred_size.width;
  float content_h = sv->content->preferred_size.height;

  if (sv->content->fill_width) {
    content_w = widget->bounds.width;
  }
  if (sv->content->fill_height) {
    content_h = widget->bounds.height;
  }

  sv->content_width = content_w;
  sv->content_height = content_h;

  float max_scroll_x = content_w - widget->bounds.width;
  float max_scroll_y = content_h - widget->bounds.height;
  if (max_scroll_x < 0) max_scroll_x = 0;
  if (max_scroll_y < 0) max_scroll_y = 0;

  if (sv->scroll_x > max_scroll_x) sv->scroll_x = max_scroll_x;
  if (sv->scroll_y > max_scroll_y) sv->scroll_y = max_scroll_y;
  if (sv->scroll_x < 0) sv->scroll_x = 0;
  if (sv->scroll_y < 0) sv->scroll_y = 0;

  zui_widget_set_bounds(sv->content,
    widget->bounds.x - sv->scroll_x,
    widget->bounds.y - sv->scroll_y,
    content_w, content_h);

  zui_widget_layout(sv->content);
}

static bool scrollview_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static ZuiWidget *scrollview_hit_test_children(ZuiWidget *widget, float x, float y)
{
  ZuiScrollView *sv = (ZuiScrollView *)widget;
  if (sv->content && sv->content->visible) {
    return zui_widget_hit_test(sv->content, x, y);
  }
  return NULL;
}

static void scrollview_on_scroll(ZuiWidget *widget, double dx, double dy)
{
  ZuiScrollView *sv = (ZuiScrollView *)widget;

  sv->scroll_x += (float)dx;
  sv->scroll_y += (float)dy;

  float max_scroll_x = sv->content_width - widget->bounds.width;
  float max_scroll_y = sv->content_height - widget->bounds.height;
  if (max_scroll_x < 0) max_scroll_x = 0;
  if (max_scroll_y < 0) max_scroll_y = 0;

  if (sv->scroll_x > max_scroll_x) sv->scroll_x = max_scroll_x;
  if (sv->scroll_y > max_scroll_y) sv->scroll_y = max_scroll_y;
  if (sv->scroll_x < 0) sv->scroll_x = 0;
  if (sv->scroll_y < 0) sv->scroll_y = 0;

  if (sv->on_scroll_cb) {
    sv->on_scroll_cb(sv, sv->scroll_x, sv->scroll_y, sv->scroll_user_data);
  }

  widget->needs_layout = true;
}

static void scrollview_destroy(ZuiWidget *widget)
{
  ZuiScrollView *sv = (ZuiScrollView *)widget;
  if (sv->content) {
    zui_widget_destroy(sv->content);
  }
}

static const ZuiWidgetVTable scrollview_vtable = {
  .draw = scrollview_draw,
  .layout = scrollview_layout,
  .hit_test = scrollview_hit_test,
  .hit_test_children = scrollview_hit_test_children,
  .on_scroll = scrollview_on_scroll,
  .destroy = scrollview_destroy,
};

ZuiScrollView *zui_scrollview_create(void)
{
  ZuiScrollView *sv = (ZuiScrollView *)zui_widget_create(
    sizeof(ZuiScrollView), ZUI_WIDGET_SCROLLVIEW, &scrollview_vtable);
  if (!sv) return NULL;

  sv->content = NULL;
  sv->scroll_x = 0;
  sv->scroll_y = 0;
  sv->content_width = 0;
  sv->content_height = 0;
  sv->on_scroll_cb = NULL;
  sv->scroll_user_data = NULL;

  sv->base.preferred_size.width = 200.0f;
  sv->base.preferred_size.height = 200.0f;

  return sv;
}

void zui_scrollview_set_content(ZuiScrollView *sv, ZuiWidget *content)
{
  if (!sv) return;
  if (sv->content) {
    zui_widget_destroy(sv->content);
  }
  sv->content = content;
  if (content) {
    content->parent = (ZuiWidget *)sv;
  }
}

void zui_scrollview_set_size(ZuiScrollView *sv, float width, float height)
{
  if (!sv) return;
  sv->base.preferred_size.width = width;
  sv->base.preferred_size.height = height;
}

void zui_scrollview_scroll_to(ZuiScrollView *sv, float x, float y)
{
  if (!sv) return;
  sv->scroll_x = x;
  sv->scroll_y = y;
  sv->base.needs_layout = true;
}

void zui_scrollview_get_scroll(ZuiScrollView *sv, float *x, float *y)
{
  if (!sv) return;
  if (x) *x = sv->scroll_x;
  if (y) *y = sv->scroll_y;
}

void zui_scrollview_on_scroll(ZuiScrollView *sv, ZuiScrollViewCallback callback,
                               void *user_data)
{
  if (!sv) return;
  sv->on_scroll_cb = callback;
  sv->scroll_user_data = user_data;
}

ZuiWidget *zui_scrollview_as_widget(ZuiScrollView *sv)
{
  return (ZuiWidget *)sv;
}

struct ZuiScroller {
  ZuiWidget base;
  bool vertical;
  float content_size;
  float viewport_size;
  float value;
  float thumb_drag_offset;
  bool dragging;
  ZuiColor track_color;
  ZuiColor thumb_color;
  ZuiColor thumb_hover_color;
  ZuiColor thumb_drag_color;
  ZuiScrollerCallback on_change;
  void *change_user_data;
};

static float scroller_get_thumb_size(ZuiScroller *sc)
{
  if (sc->content_size <= sc->viewport_size) {
    return sc->vertical ? sc->base.bounds.height : sc->base.bounds.width;
  }

  float track_size = sc->vertical ? sc->base.bounds.height : sc->base.bounds.width;
  float ratio = sc->viewport_size / sc->content_size;
  float thumb_size = track_size * ratio;

  float min_thumb = 30.0f;
  if (thumb_size < min_thumb) thumb_size = min_thumb;
  if (thumb_size > track_size) thumb_size = track_size;

  return thumb_size;
}

static float scroller_get_max_value(ZuiScroller *sc)
{
  float max_val = sc->content_size - sc->viewport_size;
  return max_val > 0 ? max_val : 0;
}

static float scroller_get_thumb_pos(ZuiScroller *sc)
{
  float max_val = scroller_get_max_value(sc);
  if (max_val <= 0) return 0;

  float track_size = sc->vertical ? sc->base.bounds.height : sc->base.bounds.width;
  float thumb_size = scroller_get_thumb_size(sc);
  float available = track_size - thumb_size;

  return (sc->value / max_val) * available;
}

static void scroller_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiScroller *sc = (ZuiScroller *)widget;

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x, widget->bounds.y,
             widget->bounds.width, widget->bounds.height),
    sc->track_color, widget->corner_radius);

  float thumb_size = scroller_get_thumb_size(sc);
  float thumb_pos = scroller_get_thumb_pos(sc);

  ZuiColor thumb_color = sc->thumb_color;
  if (sc->dragging) {
    thumb_color = sc->thumb_drag_color;
  } else if (widget->hovered) {
    thumb_color = sc->thumb_hover_color;
  }

  float thumb_x, thumb_y, thumb_w, thumb_h;
  if (sc->vertical) {
    thumb_x = widget->bounds.x + 2.0f;
    thumb_y = widget->bounds.y + thumb_pos;
    thumb_w = widget->bounds.width - 4.0f;
    thumb_h = thumb_size;
  } else {
    thumb_x = widget->bounds.x + thumb_pos;
    thumb_y = widget->bounds.y + 2.0f;
    thumb_w = thumb_size;
    thumb_h = widget->bounds.height - 4.0f;
  }

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(thumb_x, thumb_y, thumb_w, thumb_h),
    thumb_color, widget->corner_radius - 1.0f);
}

static bool scroller_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static void scroller_on_mouse_enter(ZuiWidget *widget)
{
  (void)widget;
}

static void scroller_on_mouse_leave(ZuiWidget *widget)
{
  (void)widget;
}

static void scroller_on_mouse_down(ZuiWidget *widget, float x, float y,
                                    uint32_t button)
{
  if (button != BTN_LEFT) return;

  ZuiScroller *sc = (ZuiScroller *)widget;

  float thumb_size = scroller_get_thumb_size(sc);
  float thumb_pos = scroller_get_thumb_pos(sc);

  float local_pos = sc->vertical
    ? (y - widget->bounds.y)
    : (x - widget->bounds.x);

  if (local_pos >= thumb_pos && local_pos <= thumb_pos + thumb_size) {
    sc->dragging = true;
    sc->thumb_drag_offset = local_pos - thumb_pos;
  } else {
    float track_size = sc->vertical ? widget->bounds.height : widget->bounds.width;
    float available = track_size - thumb_size;
    float max_val = scroller_get_max_value(sc);

    if (available > 0 && max_val > 0) {
      float new_thumb_pos = local_pos - thumb_size / 2;
      if (new_thumb_pos < 0) new_thumb_pos = 0;
      if (new_thumb_pos > available) new_thumb_pos = available;

      sc->value = (new_thumb_pos / available) * max_val;

      if (sc->on_change) {
        sc->on_change(sc, sc->value, sc->change_user_data);
      }
    }

    sc->dragging = true;
    sc->thumb_drag_offset = thumb_size / 2;
  }
}

static void scroller_on_mouse_up(ZuiWidget *widget, float x, float y,
                                  uint32_t button)
{
  (void)x;
  (void)y;
  if (button != BTN_LEFT) return;

  ZuiScroller *sc = (ZuiScroller *)widget;
  sc->dragging = false;
}

static void scroller_on_mouse_move(ZuiWidget *widget, float x, float y)
{
  ZuiScroller *sc = (ZuiScroller *)widget;
  if (!sc->dragging) return;

  float local_pos = sc->vertical
    ? (y - widget->bounds.y)
    : (x - widget->bounds.x);

  float thumb_size = scroller_get_thumb_size(sc);
  float track_size = sc->vertical ? widget->bounds.height : widget->bounds.width;
  float available = track_size - thumb_size;
  float max_val = scroller_get_max_value(sc);

  if (available > 0 && max_val > 0) {
    float new_thumb_pos = local_pos - sc->thumb_drag_offset;
    if (new_thumb_pos < 0) new_thumb_pos = 0;
    if (new_thumb_pos > available) new_thumb_pos = available;

    float new_value = (new_thumb_pos / available) * max_val;
    if (new_value != sc->value) {
      sc->value = new_value;
      if (sc->on_change) {
        sc->on_change(sc, sc->value, sc->change_user_data);
      }
    }
  }
}

static void scroller_destroy(ZuiWidget *widget)
{
  (void)widget;
}

static const ZuiWidgetVTable scroller_vtable = {
  .draw = scroller_draw,
  .hit_test = scroller_hit_test,
  .on_mouse_enter = scroller_on_mouse_enter,
  .on_mouse_leave = scroller_on_mouse_leave,
  .on_mouse_down = scroller_on_mouse_down,
  .on_mouse_up = scroller_on_mouse_up,
  .on_mouse_move = scroller_on_mouse_move,
  .destroy = scroller_destroy,
};

ZuiScroller *zui_scroller_create(bool vertical)
{
  ZuiScroller *sc = (ZuiScroller *)zui_widget_create(
    sizeof(ZuiScroller), ZUI_WIDGET_SCROLLER, &scroller_vtable);
  if (!sc) return NULL;

  sc->vertical = vertical;
  sc->content_size = 100.0f;
  sc->viewport_size = 100.0f;
  sc->value = 0.0f;
  sc->thumb_drag_offset = 0.0f;
  sc->dragging = false;

  sc->track_color = ZUI_COLOR_HEX(0x2d2d2d);
  sc->thumb_color = ZUI_COLOR_HEX(0x5a5a5a);
  sc->thumb_hover_color = ZUI_COLOR_HEX(0x6a6a6a);
  sc->thumb_drag_color = ZUI_COLOR_HEX(0x7a7a7a);

  sc->on_change = NULL;
  sc->change_user_data = NULL;

  sc->base.corner_radius = 4.0f;
  sc->base.cursor = ZUI_CURSOR_POINTER;

  if (vertical) {
    sc->base.preferred_size.width = 10.0f;
    sc->base.preferred_size.height = 100.0f;
  } else {
    sc->base.preferred_size.width = 100.0f;
    sc->base.preferred_size.height = 10.0f;
  }

  return sc;
}

void zui_scroller_set_size(ZuiScroller *scroller, float width, float height)
{
  if (!scroller) return;
  scroller->base.preferred_size.width = width;
  scroller->base.preferred_size.height = height;
}

void zui_scroller_set_range(ZuiScroller *scroller, float content_size,
                             float viewport_size)
{
  if (!scroller) return;
  scroller->content_size = content_size;
  scroller->viewport_size = viewport_size;

  float max_val = scroller_get_max_value(scroller);
  if (scroller->value > max_val) {
    scroller->value = max_val;
  }
}

void zui_scroller_set_value(ZuiScroller *scroller, float value)
{
  if (!scroller) return;

  float max_val = scroller_get_max_value(scroller);
  if (value < 0) value = 0;
  if (value > max_val) value = max_val;

  scroller->value = value;
}

float zui_scroller_get_value(ZuiScroller *scroller)
{
  if (!scroller) return 0.0f;
  return scroller->value;
}

void zui_scroller_on_change(ZuiScroller *scroller, ZuiScrollerCallback callback,
                             void *user_data)
{
  if (!scroller) return;
  scroller->on_change = callback;
  scroller->change_user_data = user_data;
}

ZuiWidget *zui_scroller_as_widget(ZuiScroller *scroller)
{
  return (ZuiWidget *)scroller;
}

struct ZuiSplitView {
  ZuiWidget base;
  bool vertical;
  ZuiWidget **children;
  size_t child_count;
  size_t child_capacity;
  float *positions;
  float min_child_size;
  float handle_size;
  float grip_length;
  int dragging_handle;
  float drag_offset;
  int hovered_handle;
  ZuiSplitAnchor anchor;
  float last_total;
  ZuiColor handle_color;
  ZuiColor grip_color;
  ZuiColor grip_hover_color;
  ZuiColor grip_drag_color;
};

static void splitview_layout(ZuiWidget *widget)
{
  ZuiSplitView *sv = (ZuiSplitView *)widget;

  if (sv->child_count == 0) return;

  float total = sv->vertical ? widget->bounds.height : widget->bounds.width;
  float handle = sv->handle_size;
  size_t handle_count = sv->child_count > 1 ? sv->child_count - 1 : 0;

  if (sv->child_count == 1) {
    ZuiWidget *child = sv->children[0];
    if (child) {
      zui_widget_set_bounds(child, widget->bounds.x, widget->bounds.y,
                             widget->bounds.width, widget->bounds.height);
      zui_widget_layout(child);
    }
    sv->last_total = total;
    return;
  }

  float total_handle_space = (float)handle_count * handle;
  float available = total - total_handle_space;

  if (sv->last_total > 0.0f && total != sv->last_total) {
    float old_available = sv->last_total - total_handle_space;
    float delta = available - old_available;

    switch (sv->anchor) {
      case ZUI_SPLIT_ANCHOR_START:
        break;

      case ZUI_SPLIT_ANCHOR_END:
        for (size_t i = 0; i < handle_count; i++) {
          sv->positions[i] += delta;
        }
        break;

      case ZUI_SPLIT_ANCHOR_CENTER:
        for (size_t i = 0; i < handle_count; i++) {
          sv->positions[i] += delta / 2.0f;
        }
        break;

      case ZUI_SPLIT_ANCHOR_AUTO:
      default:
        if (old_available > 0.0f) {
          float scale = available / old_available;
          for (size_t i = 0; i < handle_count; i++) {
            sv->positions[i] *= scale;
          }
        }
        break;
    }
  }

  sv->last_total = total;

  for (size_t i = 0; i < handle_count; i++) {
    float min_pos = sv->min_child_size;
    if (i > 0) {
      min_pos = sv->positions[i - 1] + handle + sv->min_child_size;
    }

    float max_pos = available;
    for (size_t j = i + 1; j < handle_count; j++) {
      max_pos -= sv->min_child_size;
    }
    max_pos -= sv->min_child_size;

    if (sv->positions[i] < min_pos) sv->positions[i] = min_pos;
    if (sv->positions[i] > max_pos) sv->positions[i] = max_pos;
  }

  float offset = 0.0f;
  for (size_t i = 0; i < sv->child_count; i++) {
    ZuiWidget *child = sv->children[i];
    if (!child) {
      if (i < handle_count) {
        offset = sv->positions[i] + handle;
      }
      continue;
    }

    float child_size;
    if (i == 0) {
      child_size = sv->positions[0];
    } else if (i < handle_count) {
      child_size = sv->positions[i] - sv->positions[i - 1] - handle;
    } else {
      child_size = available - sv->positions[i - 1];
    }

    float x, y, w, h;
    if (sv->vertical) {
      x = widget->bounds.x;
      y = widget->bounds.y + offset;
      w = widget->bounds.width;
      h = child_size;
    } else {
      x = widget->bounds.x + offset;
      y = widget->bounds.y;
      w = child_size;
      h = widget->bounds.height;
    }

    zui_widget_set_bounds(child, x, y, w, h);
    zui_widget_layout(child);

    offset += child_size + handle;
  }
}

static void splitview_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiSplitView *sv = (ZuiSplitView *)widget;

  for (size_t i = 0; i < sv->child_count; i++) {
    ZuiWidget *child = sv->children[i];
    if (child && child->visible) {
      zui_widget_draw(child, renderer);
    }
  }

  if (sv->child_count < 2) return;

  size_t handle_count = sv->child_count - 1;

  for (size_t i = 0; i < handle_count; i++) {
    float handle_pos = sv->positions[i];

    float hx, hy, hw, hh;
    if (sv->vertical) {
      hx = widget->bounds.x;
      hy = widget->bounds.y + handle_pos;
      hw = widget->bounds.width;
      hh = sv->handle_size;
    } else {
      hx = widget->bounds.x + handle_pos;
      hy = widget->bounds.y;
      hw = sv->handle_size;
      hh = widget->bounds.height;
    }

    if (sv->handle_color.a > 0.0f) {
      zui_renderer_draw_rounded_rect(renderer,
        ZUI_RECT(hx, hy, hw, hh), sv->handle_color, 0.0f);
    }

    if (sv->grip_color.a > 0.0f) {
      ZuiColor grip = sv->grip_color;
      if (sv->dragging_handle == (int)i) {
        grip = sv->grip_drag_color;
      } else if (sv->hovered_handle == (int)i) {
        grip = sv->grip_hover_color;
      }

      float gx, gy, gw, gh, gr;
      if (sv->vertical) {
        gw = sv->grip_length;
        gh = sv->handle_size - 2.0f;
        gx = hx + (hw - gw) / 2.0f;
        gy = hy + (hh - gh) / 2.0f;
        gr = gh / 2.0f;
      } else {
        gw = sv->handle_size - 2.0f;
        gh = sv->grip_length;
        gx = hx + (hw - gw) / 2.0f;
        gy = hy + (hh - gh) / 2.0f;
        gr = gw / 2.0f;
      }

      zui_renderer_draw_rounded_rect(renderer,
        ZUI_RECT(gx, gy, gw, gh), grip, gr);
    }
  }
}

static int splitview_handle_at(ZuiSplitView *sv, float x, float y)
{
  if (sv->child_count < 2) return -1;

  ZuiWidget *widget = (ZuiWidget *)sv;
  size_t handle_count = sv->child_count - 1;

  for (size_t i = 0; i < handle_count; i++) {
    float handle_pos = sv->positions[i];
    float hx, hy, hw, hh;

    if (sv->vertical) {
      hx = widget->bounds.x;
      hy = widget->bounds.y + handle_pos;
      hw = widget->bounds.width;
      hh = sv->handle_size;
    } else {
      hx = widget->bounds.x + handle_pos;
      hy = widget->bounds.y;
      hw = sv->handle_size;
      hh = widget->bounds.height;
    }

    if (x >= hx && x < hx + hw && y >= hy && y < hy + hh) {
      return (int)i;
    }
  }

  return -1;
}

static bool splitview_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static ZuiWidget *splitview_hit_test_children(ZuiWidget *widget, float x, float y)
{
  ZuiSplitView *sv = (ZuiSplitView *)widget;

  int handle = splitview_handle_at(sv, x, y);
  if (handle >= 0) {
    sv->hovered_handle = handle;
    return NULL;
  }

  sv->hovered_handle = -1;

  for (size_t i = 0; i < sv->child_count; i++) {
    ZuiWidget *child = sv->children[i];
    if (child && child->visible) {
      ZuiWidget *hit = zui_widget_hit_test(child, x, y);
      if (hit) return hit;
    }
  }

  return NULL;
}

static void splitview_on_mouse_enter(ZuiWidget *widget)
{
  (void)widget;
}

static void splitview_on_mouse_leave(ZuiWidget *widget)
{
  ZuiSplitView *sv = (ZuiSplitView *)widget;
  sv->hovered_handle = -1;
}

static void splitview_on_mouse_down(ZuiWidget *widget, float x, float y,
                                     uint32_t button)
{
  if (button != BTN_LEFT) return;

  ZuiSplitView *sv = (ZuiSplitView *)widget;

  int handle = splitview_handle_at(sv, x, y);
  if (handle >= 0) {
    sv->dragging_handle = handle;
    if (sv->vertical) {
      sv->drag_offset = y - (widget->bounds.y + sv->positions[handle]);
    } else {
      sv->drag_offset = x - (widget->bounds.x + sv->positions[handle]);
    }
  }
}

static void splitview_on_mouse_up(ZuiWidget *widget, float x, float y,
                                   uint32_t button)
{
  (void)x;
  (void)y;
  if (button != BTN_LEFT) return;

  ZuiSplitView *sv = (ZuiSplitView *)widget;
  sv->dragging_handle = -1;
}

static void splitview_on_mouse_move(ZuiWidget *widget, float x, float y)
{
  ZuiSplitView *sv = (ZuiSplitView *)widget;

  sv->hovered_handle = splitview_handle_at(sv, x, y);

  if (sv->dragging_handle < 0) return;

  int i = sv->dragging_handle;

  float new_pos;
  if (sv->vertical) {
    new_pos = y - widget->bounds.y - sv->drag_offset;
  } else {
    new_pos = x - widget->bounds.x - sv->drag_offset;
  }

  float total = sv->vertical ? widget->bounds.height : widget->bounds.width;
  size_t handle_count = sv->child_count - 1;
  float total_handle_space = (float)handle_count * sv->handle_size;
  float available = total - total_handle_space;

  float min_pos = sv->min_child_size;
  if (i > 0) {
    min_pos = sv->positions[i - 1] + sv->handle_size + sv->min_child_size;
  }

  float max_pos = available;
  for (size_t j = (size_t)i + 1; j < handle_count; j++) {
    max_pos -= sv->min_child_size;
  }
  max_pos -= sv->min_child_size;

  if (new_pos < min_pos) new_pos = min_pos;
  if (new_pos > max_pos) new_pos = max_pos;

  if (new_pos != sv->positions[i]) {
    sv->positions[i] = new_pos;
    widget->needs_layout = true;
  }
}

static void splitview_destroy(ZuiWidget *widget)
{
  ZuiSplitView *sv = (ZuiSplitView *)widget;
  for (size_t i = 0; i < sv->child_count; i++) {
    if (sv->children[i]) {
      zui_widget_destroy(sv->children[i]);
    }
  }
  free(sv->children);
  free(sv->positions);
}

static const ZuiWidgetVTable splitview_vtable = {
  .draw = splitview_draw,
  .layout = splitview_layout,
  .hit_test = splitview_hit_test,
  .hit_test_children = splitview_hit_test_children,
  .on_mouse_enter = splitview_on_mouse_enter,
  .on_mouse_leave = splitview_on_mouse_leave,
  .on_mouse_down = splitview_on_mouse_down,
  .on_mouse_up = splitview_on_mouse_up,
  .on_mouse_move = splitview_on_mouse_move,
  .destroy = splitview_destroy,
};

ZuiSplitView *zui_splitview_create(bool vertical)
{
  ZuiSplitView *sv = (ZuiSplitView *)zui_widget_create(
    sizeof(ZuiSplitView), ZUI_WIDGET_SPLITVIEW, &splitview_vtable);
  if (!sv) return NULL;

  sv->vertical = vertical;
  sv->children = NULL;
  sv->child_count = 0;
  sv->child_capacity = 0;
  sv->positions = NULL;
  sv->min_child_size = 50.0f;
  sv->handle_size = 6.0f;
  sv->dragging_handle = -1;
  sv->drag_offset = 0.0f;
  sv->hovered_handle = -1;
  sv->anchor = ZUI_SPLIT_ANCHOR_AUTO;
  sv->last_total = 0.0f;

  sv->grip_length = 40.0f;
  sv->handle_color = ZUI_COLOR(0.0f, 0.0f, 0.0f, 0.0f);
  sv->grip_color = ZUI_COLOR_HEX(0x555555);
  sv->grip_hover_color = ZUI_COLOR_HEX(0x777777);
  sv->grip_drag_color = ZUI_COLOR_HEX(0x4a9eff);

  sv->base.cursor = vertical ? ZUI_CURSOR_ROW_RESIZE : ZUI_CURSOR_COL_RESIZE;
  sv->base.preferred_size.width = 400.0f;
  sv->base.preferred_size.height = 300.0f;

  return sv;
}

void zui_splitview_set_vertical(ZuiSplitView *sv, bool vertical)
{
  if (!sv) return;
  sv->vertical = vertical;
  sv->base.cursor = vertical ? ZUI_CURSOR_ROW_RESIZE : ZUI_CURSOR_COL_RESIZE;
  sv->base.needs_layout = true;
}

void zui_splitview_add_child(ZuiSplitView *sv, ZuiWidget *widget)
{
  if (!sv || !widget) return;

  if (sv->child_count >= sv->child_capacity) {
    size_t new_cap = sv->child_capacity == 0 ? 4 : sv->child_capacity * 2;
    ZuiWidget **new_children = realloc(sv->children, new_cap * sizeof(ZuiWidget *));
    if (!new_children) return;
    sv->children = new_children;
    sv->child_capacity = new_cap;
  }

  widget->parent = (ZuiWidget *)sv;
  sv->children[sv->child_count] = widget;
  sv->child_count++;

  if (sv->child_count >= 2) {
    size_t handle_count = sv->child_count - 1;
    float *new_positions = realloc(sv->positions, handle_count * sizeof(float));
    if (new_positions) {
      sv->positions = new_positions;
      sv->positions[handle_count - 1] = 200.0f * (float)handle_count;
    }
  }

  sv->base.needs_layout = true;
}

size_t zui_splitview_get_child_count(ZuiSplitView *sv)
{
  if (!sv) return 0;
  return sv->child_count;
}

void zui_splitview_set_handle_position(ZuiSplitView *sv, size_t index, float position)
{
  if (!sv) return;
  if (sv->child_count < 2 || index >= sv->child_count - 1) return;
  sv->positions[index] = position;
  sv->base.needs_layout = true;
}

float zui_splitview_get_handle_position(ZuiSplitView *sv, size_t index)
{
  if (!sv) return 0.0f;
  if (sv->child_count < 2 || index >= sv->child_count - 1) return 0.0f;
  return sv->positions[index];
}

void zui_splitview_set_min_child_size(ZuiSplitView *sv, float min_size)
{
  if (!sv) return;
  sv->min_child_size = min_size;
  sv->base.needs_layout = true;
}

void zui_splitview_set_anchor(ZuiSplitView *sv, ZuiSplitAnchor anchor)
{
  if (!sv) return;
  sv->anchor = anchor;
}

void zui_splitview_set_handle_size(ZuiSplitView *sv, float size)
{
  if (!sv) return;
  sv->handle_size = size;
  sv->base.needs_layout = true;
}

void zui_splitview_set_grip_length(ZuiSplitView *sv, float length)
{
  if (!sv) return;
  sv->grip_length = length;
}

void zui_splitview_set_handle_color(ZuiSplitView *sv, ZuiColor color)
{
  if (!sv) return;
  sv->handle_color = color;
}

void zui_splitview_set_grip_color(ZuiSplitView *sv, ZuiColor normal,
                                   ZuiColor hover, ZuiColor drag)
{
  if (!sv) return;
  sv->grip_color = normal;
  sv->grip_hover_color = hover;
  sv->grip_drag_color = drag;
}

ZuiWidget *zui_splitview_as_widget(ZuiSplitView *sv)
{
  return (ZuiWidget *)sv;
}

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
      g_default_font = zui_font_load(font_paths[i], 16.0f);
      if (g_default_font) break;
    }
  }

  return g_default_font;
}

ZuiFont *zui_font_create(void)
{
  ZuiFont *font = NULL;
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
    font = zui_font_load(font_paths[i], 16.0f);
    if (font) break;
  }
  return font;
}

ZuiText *zui_text_create(const char *content)
{
  ZuiText *text = (ZuiText *)zui_widget_create(
    sizeof(ZuiText), ZUI_WIDGET_TEXT, &text_vtable);
  if (!text) return NULL;

  text->text = content ? strdup(content) : NULL;
  text->text_color = ZUI_COLOR_HEX(0xffffff);
  text->font = get_default_font();
  text->owns_font = false;

  if (text->font && content) {
    text->base.preferred_size.width = zui_font_text_width(text->font, content) + 16;
    text->base.preferred_size.height = zui_font_text_height(text->font, content) + 8;
  } else {
    size_t len = content ? strlen(content) : 0;
    text->base.preferred_size.width = (float)(len * 8 + 16);
    text->base.preferred_size.height = 24.0f;
  }

  return text;
}

void zui_text_set_size(ZuiText *text, float size)
{
  if (!text) return;

  if (size < 9.0f) size = 9.0f;
  else if (size > 107.0f) size = 100.0f;

  if (text->font && fabsf(text->font->size - size) < 0.5f) {
    return;
  }

  if (text->font == g_default_font) {
    text->font = zui_font_create();
    text->owns_font = true;
  }

  if (text->font) {
    zui_texture_destroy(&text->font->atlas);
    if (!init_font(text->font, text->font->font_data, size)) {
      free(text->font->font_data);
    }
  }

  if (text->font && text->text) {
    text->base.preferred_size.width = zui_font_text_width(text->font, text->text) + 16;
    text->base.preferred_size.height = zui_font_text_height(text->font, text->text) + 8;
  } else {
    size_t len = text->text ? strlen(text->text) : 0;
    text->base.preferred_size.width = (float)(len * 8 + 16);
    text->base.preferred_size.height = 24.0f;
  }
}

float zui_text_get_font_size(ZuiText *text)
{
  if (!text || !text->font) return 0.0f;
  return text->font->size;
}

void zui_text_set_content(ZuiText *text, const char *content)
{
  if (!text) return;
  free(text->text);
  text->text = content ? strdup(content) : NULL;

  if (text->font && content) {
    text->base.preferred_size.width = zui_font_text_width(text->font, content) + 16;
    text->base.preferred_size.height = zui_font_text_height(text->font, content) + 8;
  } else {
    size_t len = content ? strlen(content) : 0;
    text->base.preferred_size.width = (float)(len * 8 + 16);
    text->base.preferred_size.height = 24.0f;
  }
}

void zui_text_set_font(ZuiText *text, ZuiFont *font)
{
  if (!text) return;
  if (text->owns_font && text->font) {
    zui_font_destroy(text->font);
  }
  text->font = font;
  text->owns_font = false;

  if (text->font && text->text) {
    text->base.preferred_size.width = zui_font_text_width(text->font, text->text) + 16;
    text->base.preferred_size.height = zui_font_text_height(text->font, text->text) + 8;
  }
}

void zui_text_set_color(ZuiText *text, ZuiColor color)
{
  if (!text) return;
  text->text_color = color;
}

const char *zui_text_get_text(ZuiText *text)
{
  return text ? text->text : NULL;
}

ZuiWidget *zui_text_as_widget(ZuiText *text)
{
  return (ZuiWidget *)text;
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
  ZuiIconButton *btn = create_icon_button("res:/zui/icons/close.svg",
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
  ZuiIconButton *btn = create_icon_button("res:/zui/icons/minimize.svg",
                                           16.0f, ZUI_COLOR_HEX(0xffffff));
  if (!btn) return NULL;

  btn->base.normal_color = ZUI_COLOR_HEX(0x182449);
  btn->base.hover_color = ZUI_COLOR_HEX(0x1D2E62);
  btn->base.pressed_color = ZUI_COLOR_HEX(0x11131F);

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
  btn->base.normal_color = ZUI_COLOR_HEX(0x182449);
  btn->base.hover_color = ZUI_COLOR_HEX(0x1D2E62);
  btn->base.pressed_color = ZUI_COLOR_HEX(0x11131F);

  btn->base.base.preferred_size.width = 20.0f;
  btn->base.base.preferred_size.height = 20.0f;
  btn->base.base.corner_radius = 10.0f;
  btn->base.base.cursor = ZUI_CURSOR_POINTER;

  btn->icon_size = 12.0f;
  btn->icon_color = ZUI_COLOR_HEX(0xffffff);
  btn->window = window;

  btn->maximize_icon = load_icon_resource("res:/zui/icons/maximize.svg");
  btn->restore_icon = load_icon_resource("res:/zui/icons/restore.svg");

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
  ZuiIconButton *btn = create_icon_button("res:/zui/icons/hidden.svg",
                                           16.0f, ZUI_COLOR_HEX(0xffffff));
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

struct ZuiSlider {
  ZuiWidget base;
  float min_value;
  float max_value;
  float value;
  bool dragging;
  ZuiColor track_color;
  ZuiColor fill_color;
  ZuiColor thumb_color;
  ZuiColor thumb_hover_color;
  ZuiColor thumb_drag_color;
  float thumb_radius;
  ZuiSliderCallback on_change;
  void *change_user_data;
};

static void slider_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiSlider *sl = (ZuiSlider *)widget;

  float track_height = 6.0f;
  float track_y = widget->bounds.y + (widget->bounds.height - track_height) / 2;
  float track_radius = track_height / 2;

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x, track_y, widget->bounds.width, track_height),
    sl->track_color, track_radius);

  float range = sl->max_value - sl->min_value;
  float ratio = range > 0 ? (sl->value - sl->min_value) / range : 0;
  float fill_width = widget->bounds.width * ratio;

  if (fill_width > 0) {
    zui_renderer_draw_rounded_rect(renderer,
      ZUI_RECT(widget->bounds.x, track_y, fill_width, track_height),
      sl->fill_color, track_radius);
  }

  float thumb_x = widget->bounds.x + fill_width;
  float thumb_y = widget->bounds.y + widget->bounds.height / 2;

  ZuiColor thumb_color = sl->thumb_color;
  if (sl->dragging) {
    thumb_color = sl->thumb_drag_color;
  } else if (widget->hovered) {
    thumb_color = sl->thumb_hover_color;
  }

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(thumb_x - sl->thumb_radius, thumb_y - sl->thumb_radius,
             sl->thumb_radius * 2, sl->thumb_radius * 2),
    thumb_color, sl->thumb_radius);
}

static bool slider_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static void slider_update_value(ZuiSlider *sl, float x)
{
  float ratio = (x - sl->base.bounds.x) / sl->base.bounds.width;
  if (ratio < 0) ratio = 0;
  if (ratio > 1) ratio = 1;

  float new_value = sl->min_value + ratio * (sl->max_value - sl->min_value);
  if (new_value != sl->value) {
    sl->value = new_value;
    if (sl->on_change) {
      sl->on_change(sl, sl->value, sl->change_user_data);
    }
  }
}

static void slider_on_mouse_down(ZuiWidget *widget, float x, float y,
                                  uint32_t button)
{
  (void)y;
  if (button != BTN_LEFT) return;

  ZuiSlider *sl = (ZuiSlider *)widget;
  sl->dragging = true;
  slider_update_value(sl, x);
}

static void slider_on_mouse_up(ZuiWidget *widget, float x, float y,
                                uint32_t button)
{
  (void)x;
  (void)y;
  if (button != BTN_LEFT) return;

  ZuiSlider *sl = (ZuiSlider *)widget;
  sl->dragging = false;
}

static void slider_on_mouse_move(ZuiWidget *widget, float x, float y)
{
  (void)y;
  ZuiSlider *sl = (ZuiSlider *)widget;
  if (!sl->dragging) return;

  slider_update_value(sl, x);
}

static void slider_destroy(ZuiWidget *widget)
{
  (void)widget;
}

static const ZuiWidgetVTable slider_vtable = {
  .draw = slider_draw,
  .hit_test = slider_hit_test,
  .on_mouse_down = slider_on_mouse_down,
  .on_mouse_up = slider_on_mouse_up,
  .on_mouse_move = slider_on_mouse_move,
  .destroy = slider_destroy,
};

ZuiSlider *zui_slider_create(float min, float max, float value)
{
  ZuiSlider *sl = (ZuiSlider *)zui_widget_create(
    sizeof(ZuiSlider), ZUI_WIDGET_SLIDER, &slider_vtable);
  if (!sl) return NULL;

  sl->min_value = min;
  sl->max_value = max;
  sl->value = value;
  sl->dragging = false;

  sl->track_color = ZUI_COLOR_HEX(0x3d3d3d);
  sl->fill_color = ZUI_COLOR_HEX(0x4a9eff);
  sl->thumb_color = ZUI_COLOR_HEX(0xffffff);
  sl->thumb_hover_color = ZUI_COLOR_HEX(0xe0e0e0);
  sl->thumb_drag_color = ZUI_COLOR_HEX(0x4a9eff);
  sl->thumb_radius = 8.0f;

  sl->on_change = NULL;
  sl->change_user_data = NULL;

  sl->base.cursor = ZUI_CURSOR_POINTER;
  sl->base.preferred_size.width = 200.0f;
  sl->base.preferred_size.height = 24.0f;

  return sl;
}

void zui_slider_set_value(ZuiSlider *slider, float value)
{
  if (!slider) return;
  if (value < slider->min_value) value = slider->min_value;
  if (value > slider->max_value) value = slider->max_value;
  slider->value = value;
}

float zui_slider_get_value(ZuiSlider *slider)
{
  if (!slider) return 0.0f;
  return slider->value;
}

void zui_slider_set_range(ZuiSlider *slider, float min, float max)
{
  if (!slider) return;
  slider->min_value = min;
  slider->max_value = max;
  if (slider->value < min) slider->value = min;
  if (slider->value > max) slider->value = max;
}

void zui_slider_set_size(ZuiSlider *slider, float width, float height)
{
  if (!slider) return;
  slider->base.preferred_size.width = width;
  slider->base.preferred_size.height = height;
}

void zui_slider_set_colors(ZuiSlider *slider, ZuiColor track, ZuiColor fill,
                            ZuiColor thumb)
{
  if (!slider) return;
  slider->track_color = track;
  slider->fill_color = fill;
  slider->thumb_color = thumb;
  slider->thumb_hover_color = ZUI_COLOR(thumb.r * 0.9f, thumb.g * 0.9f,
                                         thumb.b * 0.9f, thumb.a);
  slider->thumb_drag_color = fill;
}

void zui_slider_on_change(ZuiSlider *slider, ZuiSliderCallback callback,
                           void *user_data)
{
  if (!slider) return;
  slider->on_change = callback;
  slider->change_user_data = user_data;
}

ZuiWidget *zui_slider_as_widget(ZuiSlider *slider)
{
  return (ZuiWidget *)slider;
}

#define ZUI_DROPDOWN_MAX_ITEMS 64

struct ZuiDropdown {
  ZuiWidget base;
  char *placeholder;
  char *items[ZUI_DROPDOWN_MAX_ITEMS];
  size_t item_count;
  int selected_index;
  bool open;
  int hover_index;
  ZuiFont *font;
  ZuiColor bg_color;
  ZuiColor text_color;
  ZuiColor placeholder_color;
  ZuiColor border_color;
  ZuiColor hover_color;
  ZuiColor item_bg_color;
  float corner_radius;
  float padding;
  float item_height;
  ZuiDropdownCallback on_change;
  void *change_user_data;
  ZuiIconSource *chevron_down_icon;
  ZuiIconSource *chevron_up_icon;
  ZuiTexture chevron_down_texture;
  ZuiTexture chevron_up_texture;
  float icon_size;
};

static void dropdown_draw_popup(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiDropdown *dd = (ZuiDropdown *)widget;

  if (!dd->open || dd->item_count == 0) return;

  float text_x = widget->bounds.x + dd->padding;
  float text_h = dd->font ? zui_font_text_height(dd->font, "Ay") : 14.0f;
  float list_y = widget->bounds.y + widget->bounds.height + 2;
  float list_height = dd->item_height * (float)dd->item_count;

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x, list_y,
             widget->bounds.width, list_height + 4),
    dd->border_color, dd->corner_radius);

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x + 1, list_y + 1,
             widget->bounds.width - 2, list_height + 2),
    dd->item_bg_color, dd->corner_radius - 1);

  for (size_t i = 0; i < dd->item_count; i++) {
    float item_y = list_y + 2 + dd->item_height * (float)i;

    if ((int)i == dd->hover_index) {
      zui_renderer_draw_rect(renderer,
        ZUI_RECT(widget->bounds.x + 2, item_y,
                 widget->bounds.width - 4, dd->item_height),
        dd->hover_color);
    }

    if (dd->font && dd->items[i]) {
      float item_text_y = item_y + (dd->item_height - text_h) / 2;
      ZuiColor item_color = ((int)i == dd->selected_index)
                            ? dd->text_color
                            : ZUI_COLOR(dd->text_color.r * 0.8f,
                                        dd->text_color.g * 0.8f,
                                        dd->text_color.b * 0.8f,
                                        dd->text_color.a);
      zui_font_render_text(dd->font, renderer, text_x, item_text_y,
                            dd->items[i], item_color);
    }
  }
}

static void dropdown_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiDropdown *dd = (ZuiDropdown *)widget;

  ZuiColor border = dd->border_color;
  if (dd->open || widget->hovered) {
    border = ZUI_COLOR(border.r * 1.5f, border.g * 1.5f, border.b * 1.5f, border.a);
  }

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x, widget->bounds.y,
             widget->bounds.width, widget->bounds.height),
    border, dd->corner_radius);

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x + 1, widget->bounds.y + 1,
             widget->bounds.width - 2, widget->bounds.height - 2),
    dd->bg_color, dd->corner_radius - 1);

  float text_x = widget->bounds.x + dd->padding;
  float text_h = dd->font ? zui_font_text_height(dd->font, "Ay") : 14.0f;
  float text_y = widget->bounds.y + (widget->bounds.height - text_h) / 2;

  const char *display_text = NULL;
  ZuiColor text_color = dd->placeholder_color;

  if (dd->selected_index >= 0 && dd->selected_index < (int)dd->item_count) {
    display_text = dd->items[dd->selected_index];
    text_color = dd->text_color;
  } else if (dd->placeholder) {
    display_text = dd->placeholder;
  }

  if (display_text && dd->font) {
    zui_font_render_text(dd->font, renderer, text_x, text_y,
                          display_text, text_color);
  }

  ZuiTexture *icon_tex = dd->open ? &dd->chevron_up_texture : &dd->chevron_down_texture;
  if (icon_tex && icon_tex->id) {
    float icon_x = widget->bounds.x + widget->bounds.width - dd->padding - dd->icon_size;
    float icon_y = widget->bounds.y + (widget->bounds.height - dd->icon_size) / 2;
    zui_renderer_draw_texture(renderer, icon_tex,
      ZUI_RECT(icon_x, icon_y, dd->icon_size, dd->icon_size),
      dd->text_color);
  }
}

static bool dropdown_hit_test(ZuiWidget *widget, float x, float y)
{
  ZuiDropdown *dd = (ZuiDropdown *)widget;

  if (dd->open) {
    float list_y = widget->bounds.y + widget->bounds.height + 2;
    float list_height = dd->item_height * (float)dd->item_count + 4;

    if (x >= widget->bounds.x && x < widget->bounds.x + widget->bounds.width &&
        y >= list_y && y < list_y + list_height) {
      return true;
    }
  }

  return true;
}

static void dropdown_on_mouse_down(ZuiWidget *widget, float x, float y,
                                    uint32_t button)
{
  if (button != BTN_LEFT) return;

  ZuiDropdown *dd = (ZuiDropdown *)widget;
  ZuiWindow *window = (ZuiWindow *)find_parent_window_widget(widget);

  if (dd->open) {
    float list_y = widget->bounds.y + widget->bounds.height + 2;

    if (y >= list_y && dd->item_count > 0) {
      int index = (int)((y - list_y - 2) / dd->item_height);
      if (index >= 0 && index < (int)dd->item_count) {
        dd->selected_index = index;
        if (dd->on_change) {
          dd->on_change(dd, index, dd->items[index], dd->change_user_data);
        }
      }
    }
    dd->open = false;
    if (window) {
      zui_window_clear_overlay(window, widget);
      zui_window_mark_needs_redraw(window);
    }
  } else {
    dd->open = true;
    if (window) {
      zui_window_set_overlay(window, widget);
      zui_window_mark_needs_redraw(window);
    }
  }
}

static void dropdown_on_mouse_move(ZuiWidget *widget, float x, float y)
{
  (void)x;
  ZuiDropdown *dd = (ZuiDropdown *)widget;

  if (!dd->open) {
    dd->hover_index = -1;
    return;
  }

  float list_y = widget->bounds.y + widget->bounds.height + 2;
  int old_hover = dd->hover_index;

  if (y >= list_y && dd->item_count > 0) {
    int index = (int)((y - list_y - 2) / dd->item_height);
    if (index >= 0 && index < (int)dd->item_count) {
      dd->hover_index = index;
    } else {
      dd->hover_index = -1;
    }
  } else {
    dd->hover_index = -1;
  }

  if (dd->hover_index != old_hover) {
    ZuiWindow *window = (ZuiWindow *)find_parent_window_widget(widget);
    if (window) {
      zui_window_mark_needs_redraw(window);
    }
  }
}

static void dropdown_on_mouse_leave(ZuiWidget *widget)
{
  ZuiDropdown *dd = (ZuiDropdown *)widget;
  dd->hover_index = -1;
}

static void dropdown_on_focus(ZuiWidget *widget, bool focused)
{
  if (!focused) {
    ZuiDropdown *dd = (ZuiDropdown *)widget;
    if (dd->open) {
      dd->open = false;
      ZuiWindow *window = (ZuiWindow *)find_parent_window_widget(widget);
      if (window) {
        zui_window_clear_overlay(window, widget);
        zui_window_mark_needs_redraw(window);
      }
    }
  }
}

static void dropdown_destroy(ZuiWidget *widget)
{
  ZuiDropdown *dd = (ZuiDropdown *)widget;
  free(dd->placeholder);
  for (size_t i = 0; i < dd->item_count; i++) {
    free(dd->items[i]);
  }
  if (dd->chevron_down_texture.id) {
    zui_texture_destroy(&dd->chevron_down_texture);
  }
  if (dd->chevron_up_texture.id) {
    zui_texture_destroy(&dd->chevron_up_texture);
  }
  if (dd->chevron_down_icon) {
    zui_icon_source_destroy(dd->chevron_down_icon);
  }
  if (dd->chevron_up_icon) {
    zui_icon_source_destroy(dd->chevron_up_icon);
  }
}

static const ZuiWidgetVTable dropdown_vtable = {
  .draw = dropdown_draw,
  .draw_overlay = dropdown_draw_popup,
  .hit_test = dropdown_hit_test,
  .on_mouse_down = dropdown_on_mouse_down,
  .on_mouse_move = dropdown_on_mouse_move,
  .on_mouse_leave = dropdown_on_mouse_leave,
  .on_focus = dropdown_on_focus,
  .destroy = dropdown_destroy,
};

ZuiDropdown *zui_dropdown_create(const char *placeholder)
{
  ZuiDropdown *dd = (ZuiDropdown *)zui_widget_create(
    sizeof(ZuiDropdown), ZUI_WIDGET_DROPDOWN, &dropdown_vtable);
  if (!dd) return NULL;

  dd->placeholder = placeholder ? strdup(placeholder) : NULL;
  dd->item_count = 0;
  dd->selected_index = -1;
  dd->open = false;
  dd->hover_index = -1;
  dd->font = get_default_font();

  dd->bg_color = ZUI_COLOR_HEX(0x1a1a1a);
  dd->text_color = ZUI_COLOR_HEX(0xffffff);
  dd->placeholder_color = ZUI_COLOR_HEX(0x666666);
  dd->border_color = ZUI_COLOR_HEX(0x444444);
  dd->hover_color = ZUI_COLOR_HEX(0x3d3d3d);
  dd->item_bg_color = ZUI_COLOR_HEX(0x2d2d2d);
  dd->corner_radius = 6.0f;
  dd->padding = 10.0f;
  dd->item_height = 32.0f;
  dd->icon_size = 12.0f;

  dd->chevron_down_icon = load_icon_resource("res:/zui/icons/chevron-down.svg");
  dd->chevron_up_icon = load_icon_resource("res:/zui/icons/chevron-up.svg");

  if (dd->chevron_down_icon) {
    dd->chevron_down_texture = zui_texture_create(
      dd->chevron_down_icon->raster_data,
      dd->chevron_down_icon->raster_width,
      dd->chevron_down_icon->raster_height);
  }
  if (dd->chevron_up_icon) {
    dd->chevron_up_texture = zui_texture_create(
      dd->chevron_up_icon->raster_data,
      dd->chevron_up_icon->raster_width,
      dd->chevron_up_icon->raster_height);
  }

  dd->on_change = NULL;
  dd->change_user_data = NULL;

  dd->base.cursor = ZUI_CURSOR_POINTER;
  dd->base.preferred_size.width = 200.0f;
  dd->base.preferred_size.height = 36.0f;

  return dd;
}

void zui_dropdown_add_item(ZuiDropdown *dropdown, const char *item)
{
  if (!dropdown || !item) return;
  if (dropdown->item_count >= ZUI_DROPDOWN_MAX_ITEMS) return;

  dropdown->items[dropdown->item_count] = strdup(item);
  dropdown->item_count++;
}

void zui_dropdown_clear_items(ZuiDropdown *dropdown)
{
  if (!dropdown) return;

  for (size_t i = 0; i < dropdown->item_count; i++) {
    free(dropdown->items[i]);
  }
  dropdown->item_count = 0;
  dropdown->selected_index = -1;
}

void zui_dropdown_set_selected(ZuiDropdown *dropdown, int index)
{
  if (!dropdown) return;
  if (index < -1 || index >= (int)dropdown->item_count) return;
  dropdown->selected_index = index;
}

int zui_dropdown_get_selected(ZuiDropdown *dropdown)
{
  if (!dropdown) return -1;
  return dropdown->selected_index;
}

const char *zui_dropdown_get_selected_item(ZuiDropdown *dropdown)
{
  if (!dropdown) return NULL;
  if (dropdown->selected_index < 0 ||
      dropdown->selected_index >= (int)dropdown->item_count) {
    return NULL;
  }
  return dropdown->items[dropdown->selected_index];
}

void zui_dropdown_set_size(ZuiDropdown *dropdown, float width, float height)
{
  if (!dropdown) return;
  dropdown->base.preferred_size.width = width;
  dropdown->base.preferred_size.height = height;
}

void zui_dropdown_set_colors(ZuiDropdown *dropdown, ZuiColor background,
                              ZuiColor text, ZuiColor border)
{
  if (!dropdown) return;
  dropdown->bg_color = background;
  dropdown->text_color = text;
  dropdown->border_color = border;
}

void zui_dropdown_on_change(ZuiDropdown *dropdown, ZuiDropdownCallback callback,
                             void *user_data)
{
  if (!dropdown) return;
  dropdown->on_change = callback;
  dropdown->change_user_data = user_data;
}

ZuiWidget *zui_dropdown_as_widget(ZuiDropdown *dropdown)
{
  return (ZuiWidget *)dropdown;
}

struct ZuiProgressBar {
  ZuiWidget base;
  float value;
  ZuiColor track_color;
  ZuiColor fill_color;
  float corner_radius;
};

static void progressbar_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiProgressBar *pb = (ZuiProgressBar *)widget;

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x, widget->bounds.y,
             widget->bounds.width, widget->bounds.height),
    pb->track_color, pb->corner_radius);

  float fill_width = widget->bounds.width * pb->value;
  if (fill_width > 0) {
    zui_renderer_draw_rounded_rect(renderer,
      ZUI_RECT(widget->bounds.x, widget->bounds.y,
               fill_width, widget->bounds.height),
      pb->fill_color, pb->corner_radius);
  }
}

static void progressbar_destroy(ZuiWidget *widget)
{
  (void)widget;
}

static const ZuiWidgetVTable progressbar_vtable = {
  .draw = progressbar_draw,
  .destroy = progressbar_destroy,
};

ZuiProgressBar *zui_progressbar_create(void)
{
  ZuiProgressBar *pb = (ZuiProgressBar *)zui_widget_create(
    sizeof(ZuiProgressBar), ZUI_WIDGET_PROGRESSBAR, &progressbar_vtable);
  if (!pb) return NULL;

  pb->value = 0.0f;
  pb->track_color = ZUI_COLOR_HEX(0x3d3d3d);
  pb->fill_color = ZUI_COLOR_HEX(0x4a9eff);
  pb->corner_radius = 4.0f;

  pb->base.preferred_size.width = 200.0f;
  pb->base.preferred_size.height = 8.0f;

  return pb;
}

void zui_progressbar_set_value(ZuiProgressBar *bar, float value)
{
  if (!bar) return;
  if (value < 0) value = 0;
  if (value > 1) value = 1;
  bar->value = value;
}

float zui_progressbar_get_value(ZuiProgressBar *bar)
{
  if (!bar) return 0.0f;
  return bar->value;
}

void zui_progressbar_set_size(ZuiProgressBar *bar, float width, float height)
{
  if (!bar) return;
  bar->base.preferred_size.width = width;
  bar->base.preferred_size.height = height;
}

void zui_progressbar_set_colors(ZuiProgressBar *bar, ZuiColor track,
                                 ZuiColor fill)
{
  if (!bar) return;
  bar->track_color = track;
  bar->fill_color = fill;
}

void zui_progressbar_set_corner_radius(ZuiProgressBar *bar, float radius)
{
  if (!bar) return;
  bar->corner_radius = radius;
}

ZuiWidget *zui_progressbar_as_widget(ZuiProgressBar *bar)
{
  return (ZuiWidget *)bar;
}

/* ========== GridView ========== */

#define ZUI_GRIDVIEW_MAX_CHILDREN 256

struct ZuiGridView {
  ZuiWidget base;
  ZuiWidget *children[ZUI_GRIDVIEW_MAX_CHILDREN];
  size_t child_count;
  int columns;
  float cell_width;
  float cell_height;
  float gap_x;
  float gap_y;
  float padding;
  float scroll_x;
  float scroll_y;
  float content_width;
  float content_height;
  ZuiColor bg_color;
};

static void gridview_layout_children(ZuiGridView *gv)
{
  ZuiWidget *widget = (ZuiWidget *)gv;
  if (gv->child_count == 0 || gv->columns <= 0) return;

  float available_width = widget->bounds.width - gv->padding * 2;
  float cw = gv->cell_width;
  float ch = gv->cell_height;

  if (cw <= 0) {
    cw = (available_width - gv->gap_x * (gv->columns - 1)) / gv->columns;
  }
  if (ch <= 0) {
    ch = cw;
  }

  int rows = ((int)gv->child_count + gv->columns - 1) / gv->columns;
  gv->content_width = gv->columns * cw + (gv->columns - 1) * gv->gap_x + gv->padding * 2;
  gv->content_height = rows * ch + (rows - 1) * gv->gap_y + gv->padding * 2;

  float max_scroll_x = gv->content_width - widget->bounds.width;
  float max_scroll_y = gv->content_height - widget->bounds.height;
  if (max_scroll_x < 0) max_scroll_x = 0;
  if (max_scroll_y < 0) max_scroll_y = 0;

  if (gv->scroll_x > max_scroll_x) gv->scroll_x = max_scroll_x;
  if (gv->scroll_y > max_scroll_y) gv->scroll_y = max_scroll_y;
  if (gv->scroll_x < 0) gv->scroll_x = 0;
  if (gv->scroll_y < 0) gv->scroll_y = 0;

  for (size_t i = 0; i < gv->child_count; i++) {
    int col = (int)i % gv->columns;
    int row = (int)i / gv->columns;

    float x = widget->bounds.x + gv->padding + col * (cw + gv->gap_x) - gv->scroll_x;
    float y = widget->bounds.y + gv->padding + row * (ch + gv->gap_y) - gv->scroll_y;

    zui_widget_set_bounds(gv->children[i], x, y, cw, ch);
    zui_widget_layout(gv->children[i]);
  }
}

static void gridview_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiGridView *gv = (ZuiGridView *)widget;

  if (gv->bg_color.a > 0) {
    zui_renderer_draw_rounded_rect(renderer,
      ZUI_RECT(widget->bounds.x, widget->bounds.y,
               widget->bounds.width, widget->bounds.height),
      gv->bg_color, widget->corner_radius);
  }

  zui_renderer_push_clip(renderer,
    ZUI_RECT(widget->bounds.x, widget->bounds.y,
             widget->bounds.width, widget->bounds.height),
    widget->corner_radius);

  for (size_t i = 0; i < gv->child_count; i++) {
    ZuiWidget *child = gv->children[i];
    if (child && child->visible) {
      float cx = child->bounds.x;
      float cy = child->bounds.y;
      float cw = child->bounds.width;
      float ch = child->bounds.height;

      bool visible = (cx + cw > widget->bounds.x) &&
                     (cx < widget->bounds.x + widget->bounds.width) &&
                     (cy + ch > widget->bounds.y) &&
                     (cy < widget->bounds.y + widget->bounds.height);

      if (visible) {
        zui_widget_draw(child, renderer);
      }
    }
  }

  zui_renderer_pop_clip(renderer);
}

static void gridview_layout(ZuiWidget *widget)
{
  ZuiGridView *gv = (ZuiGridView *)widget;
  gridview_layout_children(gv);
}

static bool gridview_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static ZuiWidget *gridview_hit_test_children(ZuiWidget *widget, float x, float y)
{
  ZuiGridView *gv = (ZuiGridView *)widget;

  for (int i = (int)gv->child_count - 1; i >= 0; i--) {
    ZuiWidget *child = gv->children[i];
    if (child && child->visible) {
      ZuiWidget *hit = zui_widget_hit_test(child, x, y);
      if (hit) return hit;
    }
  }
  return NULL;
}

static void gridview_on_scroll(ZuiWidget *widget, double dx, double dy)
{
  ZuiGridView *gv = (ZuiGridView *)widget;

  gv->scroll_x += (float)dx;
  gv->scroll_y += (float)dy;

  float max_scroll_x = gv->content_width - widget->bounds.width;
  float max_scroll_y = gv->content_height - widget->bounds.height;
  if (max_scroll_x < 0) max_scroll_x = 0;
  if (max_scroll_y < 0) max_scroll_y = 0;

  if (gv->scroll_x > max_scroll_x) gv->scroll_x = max_scroll_x;
  if (gv->scroll_y > max_scroll_y) gv->scroll_y = max_scroll_y;
  if (gv->scroll_x < 0) gv->scroll_x = 0;
  if (gv->scroll_y < 0) gv->scroll_y = 0;

  widget->needs_layout = true;
}

static void gridview_destroy(ZuiWidget *widget)
{
  ZuiGridView *gv = (ZuiGridView *)widget;
  for (size_t i = 0; i < gv->child_count; i++) {
    if (gv->children[i]) {
      zui_widget_destroy(gv->children[i]);
    }
  }
}

static const ZuiWidgetVTable gridview_vtable = {
  .draw = gridview_draw,
  .layout = gridview_layout,
  .hit_test = gridview_hit_test,
  .hit_test_children = gridview_hit_test_children,
  .on_scroll = gridview_on_scroll,
  .destroy = gridview_destroy,
};

ZuiGridView *zui_gridview_create(int columns)
{
  ZuiGridView *gv = (ZuiGridView *)zui_widget_create(
    sizeof(ZuiGridView), ZUI_WIDGET_GRIDVIEW, &gridview_vtable);
  if (!gv) return NULL;

  gv->child_count = 0;
  gv->columns = columns > 0 ? columns : 3;
  gv->cell_width = 0;
  gv->cell_height = 0;
  gv->gap_x = 8.0f;
  gv->gap_y = 8.0f;
  gv->padding = 8.0f;
  gv->scroll_x = 0;
  gv->scroll_y = 0;
  gv->content_width = 0;
  gv->content_height = 0;
  gv->bg_color = ZUI_COLOR(0, 0, 0, 0);

  gv->base.preferred_size.width = 300.0f;
  gv->base.preferred_size.height = 200.0f;

  return gv;
}

void zui_gridview_set_columns(ZuiGridView *gv, int columns)
{
  if (gv && columns > 0) {
    gv->columns = columns;
    gv->base.needs_layout = true;
  }
}

void zui_gridview_set_size(ZuiGridView *gv, float width, float height)
{
  if (gv) {
    gv->base.preferred_size.width = width;
    gv->base.preferred_size.height = height;
    gv->base.needs_layout = true;
  }
}

void zui_gridview_set_cell_size(ZuiGridView *gv, float width, float height)
{
  if (gv) {
    gv->cell_width = width;
    gv->cell_height = height;
    gv->base.needs_layout = true;
  }
}

void zui_gridview_set_gap(ZuiGridView *gv, float gap_x, float gap_y)
{
  if (gv) {
    gv->gap_x = gap_x;
    gv->gap_y = gap_y;
    gv->base.needs_layout = true;
  }
}

void zui_gridview_set_padding(ZuiGridView *gv, float padding)
{
  if (gv) {
    gv->padding = padding;
    gv->base.needs_layout = true;
  }
}

void zui_gridview_set_background(ZuiGridView *gv, ZuiColor color)
{
  if (gv) {
    gv->bg_color = color;
  }
}

void zui_gridview_add_child(ZuiGridView *gv, ZuiWidget *child)
{
  if (!gv || !child) return;
  if (gv->child_count >= ZUI_GRIDVIEW_MAX_CHILDREN) return;

  gv->children[gv->child_count++] = child;
  child->parent = (ZuiWidget *)gv;
  gv->base.needs_layout = true;
}

void zui_gridview_remove_child(ZuiGridView *gv, ZuiWidget *child)
{
  if (!gv || !child) return;

  for (size_t i = 0; i < gv->child_count; i++) {
    if (gv->children[i] == child) {
      child->parent = NULL;
      for (size_t j = i; j < gv->child_count - 1; j++) {
        gv->children[j] = gv->children[j + 1];
      }
      gv->child_count--;
      gv->base.needs_layout = true;
      break;
    }
  }
}

void zui_gridview_clear(ZuiGridView *gv)
{
  if (!gv) return;

  for (size_t i = 0; i < gv->child_count; i++) {
    if (gv->children[i]) {
      gv->children[i]->parent = NULL;
      zui_widget_destroy(gv->children[i]);
      gv->children[i] = NULL;
    }
  }
  gv->child_count = 0;
  gv->scroll_x = 0;
  gv->scroll_y = 0;
  gv->base.needs_layout = true;
}

void zui_gridview_scroll_to(ZuiGridView *gv, float x, float y)
{
  if (gv) {
    gv->scroll_x = x;
    gv->scroll_y = y;
    gv->base.needs_layout = true;
  }
}

void zui_gridview_get_scroll(ZuiGridView *gv, float *x, float *y)
{
  if (gv) {
    if (x) *x = gv->scroll_x;
    if (y) *y = gv->scroll_y;
  }
}

ZuiWidget *zui_gridview_as_widget(ZuiGridView *gv)
{
  return (ZuiWidget *)gv;
}

/* ========== MenuItem ========== */

struct ZuiMenuItem {
  ZuiWidget base;
  char *text;
  char *shortcut;
  bool enabled;
  bool is_separator;
  ZuiFont *font;
  ZuiColor text_color;
  ZuiColor hover_color;
  ZuiColor disabled_color;
  ZuiMenuItemCallback on_click_cb;
  void *click_user_data;
  ZuiIconSource *icon_source;
  ZuiTexture icon_texture;
  float icon_size;
};

static void menuitem_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiMenuItem *item = (ZuiMenuItem *)widget;

  if (item->is_separator) {
    float y = widget->bounds.y + widget->bounds.height / 2;
    zui_renderer_draw_rect(renderer,
      ZUI_RECT(widget->bounds.x + 8, y, widget->bounds.width - 16, 1),
      ZUI_COLOR_HEX(0x444444));
    return;
  }

  if (widget->hovered && item->enabled) {
    zui_renderer_draw_rect(renderer,
      ZUI_RECT(widget->bounds.x, widget->bounds.y,
               widget->bounds.width, widget->bounds.height),
      item->hover_color);
  }

  ZuiColor text_col = item->enabled ? item->text_color : item->disabled_color;

  float text_x = widget->bounds.x + 12;

  if (item->icon_texture.id) {
    float icon_y = widget->bounds.y + (widget->bounds.height - item->icon_size) / 2;
    zui_renderer_draw_texture(renderer, &item->icon_texture,
      ZUI_RECT(text_x, icon_y, item->icon_size, item->icon_size),
      text_col);
    text_x += item->icon_size + 8;
  }

  if (item->font && item->text) {
    float text_h = zui_font_text_height(item->font, "Ay");
    float text_y = widget->bounds.y + (widget->bounds.height - text_h) / 2;
    zui_font_render_text(item->font, renderer, text_x, text_y, item->text, text_col);
  }

  if (item->font && item->shortcut) {
    float text_h = zui_font_text_height(item->font, "Ay");
    float text_y = widget->bounds.y + (widget->bounds.height - text_h) / 2;
    float shortcut_w = zui_font_text_width(item->font, item->shortcut);
    zui_font_render_text(item->font, renderer,
      widget->bounds.x + widget->bounds.width - shortcut_w - 12, text_y,
      item->shortcut, ZUI_COLOR_HEX(0x888888));
  }
}

static void menuitem_destroy(ZuiWidget *widget)
{
  ZuiMenuItem *item = (ZuiMenuItem *)widget;
  free(item->text);
  free(item->shortcut);
  if (item->icon_texture.id) {
    zui_texture_destroy(&item->icon_texture);
  }
  if (item->icon_source) {
    zui_icon_source_destroy(item->icon_source);
  }
}

static const ZuiWidgetVTable menuitem_vtable = {
  .draw = menuitem_draw,
  .destroy = menuitem_destroy,
};

ZuiMenuItem *zui_menuitem_create(const char *text)
{
  ZuiMenuItem *item = (ZuiMenuItem *)zui_widget_create(
    sizeof(ZuiMenuItem), ZUI_WIDGET_MENUITEM, &menuitem_vtable);
  if (!item) return NULL;

  item->text = text ? strdup(text) : NULL;
  item->shortcut = NULL;
  item->enabled = true;
  item->is_separator = false;
  item->font = get_default_font();
  item->text_color = ZUI_COLOR_HEX(0xffffff);
  item->hover_color = ZUI_COLOR_HEX(0x3d3d3d);
  item->disabled_color = ZUI_COLOR_HEX(0x666666);
  item->on_click_cb = NULL;
  item->click_user_data = NULL;
  item->icon_source = NULL;
  item->icon_texture = (ZuiTexture){0};
  item->icon_size = 16.0f;

  item->base.preferred_size.width = 150.0f;
  item->base.preferred_size.height = 28.0f;

  return item;
}

static ZuiMenuItem *menuitem_create_separator(void)
{
  ZuiMenuItem *item = (ZuiMenuItem *)zui_widget_create(
    sizeof(ZuiMenuItem), ZUI_WIDGET_MENUITEM, &menuitem_vtable);
  if (!item) return NULL;

  item->text = NULL;
  item->shortcut = NULL;
  item->enabled = false;
  item->is_separator = true;
  item->font = NULL;

  item->base.preferred_size.width = 150.0f;
  item->base.preferred_size.height = 9.0f;

  return item;
}

void zui_menuitem_set_text(ZuiMenuItem *item, const char *text)
{
  if (!item) return;
  free(item->text);
  item->text = text ? strdup(text) : NULL;
}

void zui_menuitem_set_shortcut(ZuiMenuItem *item, const char *shortcut)
{
  if (!item) return;
  free(item->shortcut);
  item->shortcut = shortcut ? strdup(shortcut) : NULL;
}

void zui_menuitem_set_enabled(ZuiMenuItem *item, bool enabled)
{
  if (item) item->enabled = enabled;
}

void zui_menuitem_on_click(ZuiMenuItem *item, ZuiMenuItemCallback callback,
                            void *user_data)
{
  if (!item) return;
  item->on_click_cb = callback;
  item->click_user_data = user_data;
}

void zui_menuitem_set_icon(ZuiMenuItem *item, const char *icon_path, float size)
{
  if (!item || !icon_path) return;

  if (item->icon_texture.id) {
    zui_texture_destroy(&item->icon_texture);
  }
  if (item->icon_source) {
    zui_icon_source_destroy(item->icon_source);
  }

  item->icon_source = zui_icon_load_svg(icon_path);
  if (item->icon_source) {
    item->icon_texture = zui_texture_create(item->icon_source->raster_data,
                                             item->icon_source->raster_width,
                                             item->icon_source->raster_height);
  }
  item->icon_size = size > 0 ? size : 16.0f;
}

ZuiWidget *zui_menuitem_as_widget(ZuiMenuItem *item)
{
  return (ZuiWidget *)item;
}

/* ========== Menu ========== */

#define ZUI_MENU_MAX_ITEMS 32

struct ZuiMenu {
  ZuiWidget base;
  char *title;
  ZuiMenuItem *items[ZUI_MENU_MAX_ITEMS];
  size_t item_count;
  bool open;
  int hover_index;
  ZuiFont *font;
  ZuiColor text_color;
  ZuiColor bg_color;
  ZuiColor hover_bg_color;
  ZuiColor popup_bg_color;
  ZuiColor border_color;
  float popup_width;
};

static void menu_draw_popup(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiMenu *menu = (ZuiMenu *)widget;
  if (!menu->open || menu->item_count == 0) return;

  float popup_x = widget->bounds.x;
  float popup_y = widget->bounds.y + widget->bounds.height;
  float popup_h = 0;

  for (size_t i = 0; i < menu->item_count; i++) {
    popup_h += menu->items[i]->base.preferred_size.height;
  }

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(popup_x, popup_y, menu->popup_width, popup_h + 8),
    menu->border_color, 4.0f);

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(popup_x + 1, popup_y + 1, menu->popup_width - 2, popup_h + 6),
    menu->popup_bg_color, 3.0f);

  float item_y = popup_y + 4;
  for (size_t i = 0; i < menu->item_count; i++) {
    ZuiMenuItem *item = menu->items[i];
    zui_widget_set_bounds((ZuiWidget *)item,
      popup_x + 1, item_y, menu->popup_width - 2,
      item->base.preferred_size.height);

    item->base.hovered = ((int)i == menu->hover_index);
    zui_widget_draw((ZuiWidget *)item, renderer);

    item_y += item->base.preferred_size.height;
  }
}

static void menu_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiMenu *menu = (ZuiMenu *)widget;

  if (widget->hovered || menu->open) {
    zui_renderer_draw_rect(renderer,
      ZUI_RECT(widget->bounds.x, widget->bounds.y,
               widget->bounds.width, widget->bounds.height),
      menu->hover_bg_color);
  }

  if (menu->font && menu->title) {
    float text_h = zui_font_text_height(menu->font, "Ay");
    float text_y = widget->bounds.y + (widget->bounds.height - text_h) / 2;
    zui_font_render_text(menu->font, renderer,
      widget->bounds.x + 12, text_y, menu->title, menu->text_color);
  }
}

static bool menu_hit_test(ZuiWidget *widget, float x, float y)
{
  ZuiMenu *menu = (ZuiMenu *)widget;

  if (menu->open && menu->item_count > 0) {
    float popup_x = widget->bounds.x;
    float popup_y = widget->bounds.y + widget->bounds.height;
    float popup_h = 8;
    for (size_t i = 0; i < menu->item_count; i++) {
      popup_h += menu->items[i]->base.preferred_size.height;
    }

    if (x >= popup_x && x < popup_x + menu->popup_width &&
        y >= popup_y && y < popup_y + popup_h) {
      return true;
    }
  }

  return true;
}

static void menu_on_mouse_down(ZuiWidget *widget, float x, float y,
                                uint32_t button)
{
  if (button != BTN_LEFT) return;

  ZuiMenu *menu = (ZuiMenu *)widget;
  ZuiWindow *window = (ZuiWindow *)find_parent_window_widget(widget);

  if (menu->open) {
    float popup_y = widget->bounds.y + widget->bounds.height;

    if (y >= popup_y && menu->item_count > 0) {
      float item_y = popup_y + 4;
      for (size_t i = 0; i < menu->item_count; i++) {
        ZuiMenuItem *item = menu->items[i];
        float item_h = item->base.preferred_size.height;

        if (y >= item_y && y < item_y + item_h) {
          if (item->enabled && !item->is_separator && item->on_click_cb) {
            item->on_click_cb(item, item->click_user_data);
          }
          break;
        }
        item_y += item_h;
      }
    }

    menu->open = false;
    menu->hover_index = -1;
    if (window) {
      zui_window_clear_overlay(window, widget);
      zui_window_mark_needs_redraw(window);
    }
  } else {
    menu->open = true;
    if (window) {
      zui_window_set_overlay(window, widget);
      zui_window_mark_needs_redraw(window);
    }
  }
}

static void menu_on_mouse_move(ZuiWidget *widget, float x, float y)
{
  (void)x;
  ZuiMenu *menu = (ZuiMenu *)widget;

  if (!menu->open) {
    menu->hover_index = -1;
    return;
  }

  float popup_y = widget->bounds.y + widget->bounds.height;
  int old_hover = menu->hover_index;
  menu->hover_index = -1;

  if (y >= popup_y && menu->item_count > 0) {
    float item_y = popup_y + 4;
    for (size_t i = 0; i < menu->item_count; i++) {
      ZuiMenuItem *item = menu->items[i];
      float item_h = item->base.preferred_size.height;

      if (y >= item_y && y < item_y + item_h) {
        if (!item->is_separator) {
          menu->hover_index = (int)i;
        }
        break;
      }
      item_y += item_h;
    }
  }

  if (menu->hover_index != old_hover) {
    ZuiWindow *window = (ZuiWindow *)find_parent_window_widget(widget);
    if (window) {
      zui_window_mark_needs_redraw(window);
    }
  }
}

static void menu_on_mouse_leave(ZuiWidget *widget)
{
  ZuiMenu *menu = (ZuiMenu *)widget;
  if (!menu->open) {
    menu->hover_index = -1;
  }
}

static void menu_on_focus(ZuiWidget *widget, bool focused)
{
  if (!focused) {
    ZuiMenu *menu = (ZuiMenu *)widget;
    if (menu->open) {
      menu->open = false;
      menu->hover_index = -1;
      ZuiWindow *window = (ZuiWindow *)find_parent_window_widget(widget);
      if (window) {
        zui_window_clear_overlay(window, widget);
        zui_window_mark_needs_redraw(window);
      }
    }
  }
}

static void menu_destroy(ZuiWidget *widget)
{
  ZuiMenu *menu = (ZuiMenu *)widget;
  free(menu->title);
  for (size_t i = 0; i < menu->item_count; i++) {
    zui_widget_destroy((ZuiWidget *)menu->items[i]);
  }
}

static const ZuiWidgetVTable menu_vtable = {
  .draw = menu_draw,
  .draw_overlay = menu_draw_popup,
  .hit_test = menu_hit_test,
  .on_mouse_down = menu_on_mouse_down,
  .on_mouse_move = menu_on_mouse_move,
  .on_mouse_leave = menu_on_mouse_leave,
  .on_focus = menu_on_focus,
  .destroy = menu_destroy,
};

ZuiMenu *zui_menu_create(const char *title)
{
  ZuiMenu *menu = (ZuiMenu *)zui_widget_create(
    sizeof(ZuiMenu), ZUI_WIDGET_MENU, &menu_vtable);
  if (!menu) return NULL;

  menu->title = title ? strdup(title) : NULL;
  menu->item_count = 0;
  menu->open = false;
  menu->hover_index = -1;
  menu->font = get_default_font();
  menu->text_color = ZUI_COLOR_HEX(0xffffff);
  menu->bg_color = ZUI_COLOR(0, 0, 0, 0);
  menu->hover_bg_color = ZUI_COLOR_HEX(0x3d3d3d);
  menu->popup_bg_color = ZUI_COLOR_HEX(0x252525);
  menu->border_color = ZUI_COLOR_HEX(0x444444);
  menu->popup_width = 180.0f;

  float title_w = 60.0f;
  if (menu->font && title) {
    title_w = zui_font_text_width(menu->font, title) + 24;
  }
  menu->base.preferred_size.width = title_w;
  menu->base.preferred_size.height = 28.0f;

  return menu;
}

void zui_menu_set_title(ZuiMenu *menu, const char *title)
{
  if (!menu) return;
  free(menu->title);
  menu->title = title ? strdup(title) : NULL;

  if (menu->font && title) {
    menu->base.preferred_size.width = zui_font_text_width(menu->font, title) + 24;
  }
}

void zui_menu_add_item(ZuiMenu *menu, ZuiMenuItem *item)
{
  if (!menu || !item) return;
  if (menu->item_count >= ZUI_MENU_MAX_ITEMS) return;

  menu->items[menu->item_count++] = item;
  item->base.parent = (ZuiWidget *)menu;

  float max_w = menu->popup_width;
  if (item->font && item->text) {
    float text_w = zui_font_text_width(item->font, item->text) + 24;
    if (item->shortcut) {
      text_w += zui_font_text_width(item->font, item->shortcut) + 24;
    }
    if (text_w > max_w) max_w = text_w;
  }
  menu->popup_width = max_w;
}

void zui_menu_add_separator(ZuiMenu *menu)
{
  if (!menu) return;
  ZuiMenuItem *sep = menuitem_create_separator();
  if (sep) {
    zui_menu_add_item(menu, sep);
  }
}

ZuiWidget *zui_menu_as_widget(ZuiMenu *menu)
{
  return (ZuiWidget *)menu;
}

/* ========== MenuBar ========== */

#define ZUI_MENUBAR_MAX_MENUS 16

struct ZuiMenuBar {
  ZuiWidget base;
  ZuiMenu *menus[ZUI_MENUBAR_MAX_MENUS];
  size_t menu_count;
  ZuiColor bg_color;
  ZuiColor text_color;
};

static void menubar_layout(ZuiWidget *widget)
{
  ZuiMenuBar *bar = (ZuiMenuBar *)widget;

  float x = widget->bounds.x;
  for (size_t i = 0; i < bar->menu_count; i++) {
    ZuiMenu *menu = bar->menus[i];
    float menu_w = menu->base.preferred_size.width;
    zui_widget_set_bounds((ZuiWidget *)menu,
      x, widget->bounds.y, menu_w, widget->bounds.height);
    x += menu_w;
  }
}

static void menubar_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiMenuBar *bar = (ZuiMenuBar *)widget;

  if (bar->bg_color.a > 0) {
    zui_renderer_draw_rect(renderer,
      ZUI_RECT(widget->bounds.x, widget->bounds.y,
               widget->bounds.width, widget->bounds.height),
      bar->bg_color);
  }

  for (size_t i = 0; i < bar->menu_count; i++) {
    zui_widget_draw((ZuiWidget *)bar->menus[i], renderer);
  }
}

static void menubar_draw_overlay(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiMenuBar *bar = (ZuiMenuBar *)widget;

  for (size_t i = 0; i < bar->menu_count; i++) {
    ZuiMenu *menu = bar->menus[i];
    if (menu->base.vtable && menu->base.vtable->draw_overlay) {
      menu->base.vtable->draw_overlay((ZuiWidget *)menu, renderer);
    }
  }
}

static ZuiWidget *menubar_hit_test_children(ZuiWidget *widget, float x, float y)
{
  ZuiMenuBar *bar = (ZuiMenuBar *)widget;

  for (size_t i = 0; i < bar->menu_count; i++) {
    ZuiMenu *menu = bar->menus[i];
    if (zui_widget_contains_point((ZuiWidget *)menu, x, y)) {
      return (ZuiWidget *)menu;
    }
    if (menu->open && menu->base.vtable->hit_test &&
        menu->base.vtable->hit_test((ZuiWidget *)menu, x, y)) {
      return (ZuiWidget *)menu;
    }
  }
  return NULL;
}

static void menubar_destroy(ZuiWidget *widget)
{
  ZuiMenuBar *bar = (ZuiMenuBar *)widget;
  for (size_t i = 0; i < bar->menu_count; i++) {
    zui_widget_destroy((ZuiWidget *)bar->menus[i]);
  }
}

static const ZuiWidgetVTable menubar_vtable = {
  .draw = menubar_draw,
  .draw_overlay = menubar_draw_overlay,
  .layout = menubar_layout,
  .hit_test_children = menubar_hit_test_children,
  .destroy = menubar_destroy,
};

ZuiMenuBar *zui_menubar_create(void)
{
  ZuiMenuBar *bar = (ZuiMenuBar *)zui_widget_create(
    sizeof(ZuiMenuBar), ZUI_WIDGET_MENUBAR, &menubar_vtable);
  if (!bar) return NULL;

  bar->menu_count = 0;
  bar->bg_color = ZUI_COLOR_HEX(0x1a1a1a);
  bar->text_color = ZUI_COLOR_HEX(0xffffff);

  bar->base.preferred_size.width = 400.0f;
  bar->base.preferred_size.height = 28.0f;

  return bar;
}

void zui_menubar_set_size(ZuiMenuBar *bar, float width, float height)
{
  if (bar) {
    bar->base.preferred_size.width = width;
    bar->base.preferred_size.height = height;
    bar->base.needs_layout = true;
  }
}

void zui_menubar_set_colors(ZuiMenuBar *bar, ZuiColor background, ZuiColor text)
{
  if (bar) {
    bar->bg_color = background;
    bar->text_color = text;
  }
}

void zui_menubar_add_menu(ZuiMenuBar *bar, ZuiMenu *menu)
{
  if (!bar || !menu) return;
  if (bar->menu_count >= ZUI_MENUBAR_MAX_MENUS) return;

  bar->menus[bar->menu_count++] = menu;
  menu->base.parent = (ZuiWidget *)bar;
  bar->base.needs_layout = true;
}

ZuiWidget *zui_menubar_as_widget(ZuiMenuBar *bar)
{
  return (ZuiWidget *)bar;
}

/* ========== PieChart ========== */

#define ZUI_PIECHART_MAX_SLICES 16
#define ZUI_PI 3.14159265358979323846f

typedef void (*ZuiPieChartCallback)(ZuiPieChart *chart, int slice_index,
                                     void *user_data);

struct ZuiPieChart {
  ZuiWidget base;
  float values[ZUI_PIECHART_MAX_SLICES];
  ZuiColor colors[ZUI_PIECHART_MAX_SLICES];
  char *texts[ZUI_PIECHART_MAX_SLICES];
  size_t slice_count;
  float hole_radius;
  bool show_texts;
  bool show_values;
  int hovered_slice;
  ZuiFont *font;
  ZuiPieChartCallback on_hover_cb;
  void *hover_user_data;
  ZuiPieChartCallback on_click_cb;
  void *click_user_data;
};

static void piechart_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiPieChart *chart = (ZuiPieChart *)widget;

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x, widget->bounds.y,
             widget->bounds.width, widget->bounds.height),
    ZUI_COLOR_HEX(0x222222), 4.0f);

  if (chart->slice_count == 0) return;

  float cx = widget->bounds.x + widget->bounds.width / 2;
  float cy = widget->bounds.y + widget->bounds.height / 2;
  float radius = (widget->bounds.width < widget->bounds.height ?
                  widget->bounds.width : widget->bounds.height) / 2 - 4;

  float total = 0;
  for (size_t i = 0; i < chart->slice_count; i++) {
    total += chart->values[i];
  }
  if (total <= 0) return;

  float start_angle = -ZUI_PI / 2;
  for (size_t i = 0; i < chart->slice_count; i++) {
    float sweep = (chart->values[i] / total) * 2 * ZUI_PI;
    float end_angle = start_angle + sweep;

    ZuiColor color = chart->colors[i];
    float draw_radius = radius;

    if ((int)i == chart->hovered_slice) {
      color = ZUI_COLOR(
        color.r * 1.2f > 1.0f ? 1.0f : color.r * 1.2f,
        color.g * 1.2f > 1.0f ? 1.0f : color.g * 1.2f,
        color.b * 1.2f > 1.0f ? 1.0f : color.b * 1.2f,
        color.a);
      draw_radius = radius + 4;
    }

    if (chart->hole_radius > 0) {
      zui_renderer_draw_arc_outline(renderer, cx, cy, draw_radius,
        start_angle, end_angle, draw_radius - chart->hole_radius, color);
    } else {
      zui_renderer_draw_arc(renderer, cx, cy, draw_radius,
        start_angle, end_angle, color);
    }

    if ((chart->show_texts || chart->show_values) && chart->font) {
      float mid_angle = start_angle + sweep / 2;
      float text_radius = (chart->hole_radius > 0)
        ? (radius + chart->hole_radius) / 2
        : radius * 0.65f;

      float lx = cx + cosf(mid_angle) * text_radius;
      float ly = cy + sinf(mid_angle) * text_radius;

      char text_buf[64];
      if (chart->show_values && chart->texts[i] && chart->show_texts) {
        snprintf(text_buf, sizeof(text_buf), "%s\n%.0f%%",
                 chart->texts[i], (chart->values[i] / total) * 100);
      } else if (chart->show_values) {
        snprintf(text_buf, sizeof(text_buf), "%.0f%%",
                 (chart->values[i] / total) * 100);
      } else if (chart->texts[i]) {
        snprintf(text_buf, sizeof(text_buf), "%s", chart->texts[i]);
      } else {
        text_buf[0] = '\0';
      }

      if (text_buf[0]) {
        float tw = zui_font_text_width(chart->font, text_buf);
        float th = zui_font_text_height(chart->font, text_buf);
        zui_font_render_text(chart->font, renderer,
          lx - tw / 2, ly - th / 2, text_buf, ZUI_COLOR_HEX(0xffffff));
      }
    }

    start_angle = end_angle;
  }
}

static int piechart_slice_at(ZuiPieChart *chart, float x, float y)
{
  ZuiWidget *widget = (ZuiWidget *)chart;
  if (chart->slice_count == 0) return -1;

  float cx = widget->bounds.x + widget->bounds.width / 2;
  float cy = widget->bounds.y + widget->bounds.height / 2;
  float radius = (widget->bounds.width < widget->bounds.height ?
                  widget->bounds.width : widget->bounds.height) / 2 - 4;

  float dx = x - cx;
  float dy = y - cy;
  float dist = sqrtf(dx * dx + dy * dy);

  if (dist > radius) return -1;
  if (chart->hole_radius > 0 && dist < chart->hole_radius) return -1;

  float angle = atan2f(dy, dx);
  if (angle < 0) angle += 2 * ZUI_PI;

  float total = 0;
  for (size_t i = 0; i < chart->slice_count; i++) {
    total += chart->values[i];
  }
  if (total <= 0) return -1;

  float start_angle = -ZUI_PI / 2;
  if (start_angle < 0) start_angle += 2 * ZUI_PI;

  for (size_t i = 0; i < chart->slice_count; i++) {
    float sweep = (chart->values[i] / total) * 2 * ZUI_PI;
    float end_angle = start_angle + sweep;

    float s = fmodf(start_angle, 2 * ZUI_PI);
    float e = fmodf(end_angle, 2 * ZUI_PI);
    if (s < 0) s += 2 * ZUI_PI;
    if (e < 0) e += 2 * ZUI_PI;

    bool in_slice;
    if (s <= e) {
      in_slice = (angle >= s && angle <= e);
    } else {
      in_slice = (angle >= s || angle <= e);
    }

    if (in_slice) return (int)i;
    start_angle = end_angle;
  }

  return -1;
}

static bool piechart_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static void piechart_on_mouse_move(ZuiWidget *widget, float x, float y)
{
  ZuiPieChart *chart = (ZuiPieChart *)widget;
  int old_hover = chart->hovered_slice;
  chart->hovered_slice = piechart_slice_at(chart, x, y);

  if (chart->hovered_slice != old_hover && chart->on_hover_cb) {
    chart->on_hover_cb(chart, chart->hovered_slice, chart->hover_user_data);
  }
}

static void piechart_on_mouse_leave(ZuiWidget *widget)
{
  ZuiPieChart *chart = (ZuiPieChart *)widget;
  if (chart->hovered_slice >= 0 && chart->on_hover_cb) {
    chart->on_hover_cb(chart, -1, chart->hover_user_data);
  }
  chart->hovered_slice = -1;
}

static void piechart_on_mouse_up(ZuiWidget *widget, float x, float y, uint32_t button)
{
  if (button != BTN_LEFT) return;

  ZuiPieChart *chart = (ZuiPieChart *)widget;
  int slice = piechart_slice_at(chart, x, y);
  if (slice >= 0 && chart->on_click_cb) {
    chart->on_click_cb(chart, slice, chart->click_user_data);
  }
}

static void piechart_destroy(ZuiWidget *widget)
{
  ZuiPieChart *chart = (ZuiPieChart *)widget;
  for (size_t i = 0; i < chart->slice_count; i++) {
    free(chart->texts[i]);
  }
}

static const ZuiWidgetVTable piechart_vtable = {
  .draw = piechart_draw,
  .hit_test = piechart_hit_test,
  .on_mouse_move = piechart_on_mouse_move,
  .on_mouse_leave = piechart_on_mouse_leave,
  .on_mouse_up = piechart_on_mouse_up,
  .destroy = piechart_destroy,
};

ZuiPieChart *zui_piechart_create(void)
{
  ZuiPieChart *chart = (ZuiPieChart *)zui_widget_create(
    sizeof(ZuiPieChart), ZUI_WIDGET_PIE_CHART, &piechart_vtable);
  if (!chart) return NULL;

  chart->slice_count = 0;
  chart->hole_radius = 0;
  chart->show_texts = false;
  chart->show_values = false;
  chart->hovered_slice = -1;
  chart->font = get_default_font();
  chart->on_hover_cb = NULL;
  chart->hover_user_data = NULL;
  chart->on_click_cb = NULL;
  chart->click_user_data = NULL;
  chart->base.cursor = ZUI_CURSOR_POINTER;
  chart->base.preferred_size.width = 100.0f;
  chart->base.preferred_size.height = 100.0f;

  for (size_t i = 0; i < ZUI_PIECHART_MAX_SLICES; i++) {
    chart->texts[i] = NULL;
  }

  return chart;
}

void zui_piechart_set_size(ZuiPieChart *chart, float size)
{
  if (chart) {
    chart->base.preferred_size.width = size;
    chart->base.preferred_size.height = size;
  }
}

void zui_piechart_add_slice(ZuiPieChart *chart, float value, ZuiColor color)
{
  if (!chart || chart->slice_count >= ZUI_PIECHART_MAX_SLICES) return;
  chart->values[chart->slice_count] = value;
  chart->colors[chart->slice_count] = color;
  chart->texts[chart->slice_count] = NULL;
  chart->slice_count++;
}

void zui_piechart_add_slice_texted(ZuiPieChart *chart, float value,
                                     ZuiColor color, const char *text)
{
  if (!chart || chart->slice_count >= ZUI_PIECHART_MAX_SLICES) return;
  chart->values[chart->slice_count] = value;
  chart->colors[chart->slice_count] = color;
  chart->texts[chart->slice_count] = text ? strdup(text) : NULL;
  chart->slice_count++;
}

void zui_piechart_clear(ZuiPieChart *chart)
{
  if (!chart) return;
  for (size_t i = 0; i < chart->slice_count; i++) {
    free(chart->texts[i]);
    chart->texts[i] = NULL;
  }
  chart->slice_count = 0;
}

void zui_piechart_set_hole_radius(ZuiPieChart *chart, float radius)
{
  if (chart) chart->hole_radius = radius;
}

void zui_piechart_set_show_texts(ZuiPieChart *chart, bool show)
{
  if (chart) chart->show_texts = show;
}

void zui_piechart_set_show_values(ZuiPieChart *chart, bool show)
{
  if (chart) chart->show_values = show;
}

void zui_piechart_on_hover(ZuiPieChart *chart, ZuiPieChartCallback callback,
                           void *user_data)
{
  if (!chart) return;
  chart->on_hover_cb = callback;
  chart->hover_user_data = user_data;
}

void zui_piechart_on_click(ZuiPieChart *chart, ZuiPieChartCallback callback,
                           void *user_data)
{
  if (!chart) return;
  chart->on_click_cb = callback;
  chart->click_user_data = user_data;
}

int zui_piechart_get_hovered_slice(ZuiPieChart *chart)
{
  if (!chart) return -1;
  return chart->hovered_slice;
}

ZuiWidget *zui_piechart_as_widget(ZuiPieChart *chart)
{
  return (ZuiWidget *)chart;
}

/* ========== BarChart ========== */

#define ZUI_BARCHART_MAX_BARS 32

typedef void (*ZuiBarChartCallback)(ZuiBarChart *chart, int bar_index,
                                     void *user_data);

struct ZuiBarChart {
  ZuiWidget base;
  float values[ZUI_BARCHART_MAX_BARS];
  ZuiColor colors[ZUI_BARCHART_MAX_BARS];
  char *texts[ZUI_BARCHART_MAX_BARS];
  size_t bar_count;
  float max_value;
  float bar_spacing;
  float corner_radius;
  bool show_texts;
  bool show_values;
  int hovered_bar;
  ZuiFont *font;
  ZuiBarChartCallback on_hover_cb;
  void *hover_user_data;
  ZuiBarChartCallback on_click_cb;
  void *click_user_data;
};

static void barchart_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiBarChart *chart = (ZuiBarChart *)widget;

  zui_renderer_draw_rounded_rect(renderer,
    ZUI_RECT(widget->bounds.x, widget->bounds.y,
             widget->bounds.width, widget->bounds.height),
    ZUI_COLOR_HEX(0x222222), 4.0f);

  if (chart->bar_count == 0) return;

  float max_val = chart->max_value;
  if (max_val <= 0) {
    for (size_t i = 0; i < chart->bar_count; i++) {
      if (chart->values[i] > max_val) max_val = chart->values[i];
    }
  }
  if (max_val <= 0) return;

  float total_spacing = chart->bar_spacing * (float)(chart->bar_count - 1);
  float bar_width = (widget->bounds.width - total_spacing) / (float)chart->bar_count;

  for (size_t i = 0; i < chart->bar_count; i++) {
    float bar_height = (chart->values[i] / max_val) * widget->bounds.height;
    float x = widget->bounds.x + (float)i * (bar_width + chart->bar_spacing);
    float y = widget->bounds.y + widget->bounds.height - bar_height;

    ZuiColor color = chart->colors[i];
    if ((int)i == chart->hovered_bar) {
      color = ZUI_COLOR(
        color.r * 1.2f > 1.0f ? 1.0f : color.r * 1.2f,
        color.g * 1.2f > 1.0f ? 1.0f : color.g * 1.2f,
        color.b * 1.2f > 1.0f ? 1.0f : color.b * 1.2f,
        color.a);
    }

    if (chart->corner_radius > 0) {
      zui_renderer_draw_rounded_rect(renderer,
        ZUI_RECT(x, y, bar_width, bar_height),
        color, chart->corner_radius);
    } else {
      zui_renderer_draw_rect(renderer,
        ZUI_RECT(x, y, bar_width, bar_height),
        color);
    }

    if ((chart->show_texts || chart->show_values) && chart->font) {
      char text_buf[64];
      if (chart->show_values && chart->texts[i] && chart->show_texts) {
        snprintf(text_buf, sizeof(text_buf), "%.0f\n%s",
                 (double)chart->values[i], chart->texts[i]);
      } else if (chart->show_values) {
        snprintf(text_buf, sizeof(text_buf), "%.0f", (double)chart->values[i]);
      } else if (chart->texts[i]) {
        snprintf(text_buf, sizeof(text_buf), "%s", chart->texts[i]);
      } else {
        text_buf[0] = '\0';
      }

      if (text_buf[0]) {
        float tw = zui_font_text_width(chart->font, text_buf);
        float th = zui_font_text_height(chart->font, text_buf);
        float lx = x + (bar_width - tw) / 2;
        float ly = y - th - 2;
        if (ly < widget->bounds.y) ly = y + 2;
        zui_font_render_text(chart->font, renderer,
          lx, ly, text_buf, ZUI_COLOR_HEX(0xffffff));
      }
    }
  }
}

static int barchart_bar_at(ZuiBarChart *chart, float x, float y)
{
  ZuiWidget *widget = (ZuiWidget *)chart;
  if (chart->bar_count == 0) return -1;

  float max_val = chart->max_value;
  if (max_val <= 0) {
    for (size_t i = 0; i < chart->bar_count; i++) {
      if (chart->values[i] > max_val) max_val = chart->values[i];
    }
  }
  if (max_val <= 0) return -1;

  float total_spacing = chart->bar_spacing * (float)(chart->bar_count - 1);
  float bar_width = (widget->bounds.width - total_spacing) / (float)chart->bar_count;

  for (size_t i = 0; i < chart->bar_count; i++) {
    float bar_height = (chart->values[i] / max_val) * widget->bounds.height;
    float bx = widget->bounds.x + (float)i * (bar_width + chart->bar_spacing);
    float by = widget->bounds.y + widget->bounds.height - bar_height;

    if (x >= bx && x < bx + bar_width && y >= by && y < by + bar_height) {
      return (int)i;
    }
  }

  return -1;
}

static bool barchart_hit_test(ZuiWidget *widget, float x, float y)
{
  (void)widget;
  (void)x;
  (void)y;
  return true;
}

static void barchart_on_mouse_move(ZuiWidget *widget, float x, float y)
{
  ZuiBarChart *chart = (ZuiBarChart *)widget;
  int old_hover = chart->hovered_bar;
  chart->hovered_bar = barchart_bar_at(chart, x, y);

  if (chart->hovered_bar != old_hover && chart->on_hover_cb) {
    chart->on_hover_cb(chart, chart->hovered_bar, chart->hover_user_data);
  }
}

static void barchart_on_mouse_leave(ZuiWidget *widget)
{
  ZuiBarChart *chart = (ZuiBarChart *)widget;
  if (chart->hovered_bar >= 0 && chart->on_hover_cb) {
    chart->on_hover_cb(chart, -1, chart->hover_user_data);
  }
  chart->hovered_bar = -1;
}

static void barchart_on_mouse_up(ZuiWidget *widget, float x, float y, uint32_t button)
{
  if (button != BTN_LEFT) return;

  ZuiBarChart *chart = (ZuiBarChart *)widget;
  int bar = barchart_bar_at(chart, x, y);
  if (bar >= 0 && chart->on_click_cb) {
    chart->on_click_cb(chart, bar, chart->click_user_data);
  }
}

static void barchart_destroy(ZuiWidget *widget)
{
  ZuiBarChart *chart = (ZuiBarChart *)widget;
  for (size_t i = 0; i < chart->bar_count; i++) {
    free(chart->texts[i]);
  }
}

static const ZuiWidgetVTable barchart_vtable = {
  .draw = barchart_draw,
  .hit_test = barchart_hit_test,
  .on_mouse_move = barchart_on_mouse_move,
  .on_mouse_leave = barchart_on_mouse_leave,
  .on_mouse_up = barchart_on_mouse_up,
  .destroy = barchart_destroy,
};

ZuiBarChart *zui_barchart_create(void)
{
  ZuiBarChart *chart = (ZuiBarChart *)zui_widget_create(
    sizeof(ZuiBarChart), ZUI_WIDGET_BAR_CHART, &barchart_vtable);
  if (!chart) return NULL;

  chart->bar_count = 0;
  chart->max_value = 0;
  chart->bar_spacing = 4.0f;
  chart->corner_radius = 3.0f;
  chart->show_texts = false;
  chart->show_values = false;
  chart->hovered_bar = -1;
  chart->font = get_default_font();
  chart->on_hover_cb = NULL;
  chart->hover_user_data = NULL;
  chart->on_click_cb = NULL;
  chart->click_user_data = NULL;
  chart->base.cursor = ZUI_CURSOR_POINTER;
  chart->base.preferred_size.width = 200.0f;
  chart->base.preferred_size.height = 100.0f;

  for (size_t i = 0; i < ZUI_BARCHART_MAX_BARS; i++) {
    chart->texts[i] = NULL;
  }

  return chart;
}

void zui_barchart_set_size(ZuiBarChart *chart, float width, float height)
{
  if (chart) {
    chart->base.preferred_size.width = width;
    chart->base.preferred_size.height = height;
  }
}

void zui_barchart_add_bar(ZuiBarChart *chart, float value, ZuiColor color)
{
  if (!chart || chart->bar_count >= ZUI_BARCHART_MAX_BARS) return;
  chart->values[chart->bar_count] = value;
  chart->colors[chart->bar_count] = color;
  chart->texts[chart->bar_count] = NULL;
  chart->bar_count++;
}

void zui_barchart_add_bar_texted(ZuiBarChart *chart, float value,
                                   ZuiColor color, const char *text)
{
  if (!chart || chart->bar_count >= ZUI_BARCHART_MAX_BARS) return;
  chart->values[chart->bar_count] = value;
  chart->colors[chart->bar_count] = color;
  chart->texts[chart->bar_count] = text ? strdup(text) : NULL;
  chart->bar_count++;
}

void zui_barchart_clear(ZuiBarChart *chart)
{
  if (!chart) return;
  for (size_t i = 0; i < chart->bar_count; i++) {
    free(chart->texts[i]);
    chart->texts[i] = NULL;
  }
  chart->bar_count = 0;
}

void zui_barchart_set_max_value(ZuiBarChart *chart, float max)
{
  if (chart) chart->max_value = max;
}

void zui_barchart_set_bar_spacing(ZuiBarChart *chart, float spacing)
{
  if (chart) chart->bar_spacing = spacing;
}

void zui_barchart_set_corner_radius(ZuiBarChart *chart, float radius)
{
  if (chart) chart->corner_radius = radius;
}

void zui_barchart_set_show_texts(ZuiBarChart *chart, bool show)
{
  if (chart) chart->show_texts = show;
}

void zui_barchart_set_show_values(ZuiBarChart *chart, bool show)
{
  if (chart) chart->show_values = show;
}

void zui_barchart_on_hover(ZuiBarChart *chart, ZuiBarChartCallback callback,
                           void *user_data)
{
  if (!chart) return;
  chart->on_hover_cb = callback;
  chart->hover_user_data = user_data;
}

void zui_barchart_on_click(ZuiBarChart *chart, ZuiBarChartCallback callback,
                           void *user_data)
{
  if (!chart) return;
  chart->on_click_cb = callback;
  chart->click_user_data = user_data;
}

int zui_barchart_get_hovered_bar(ZuiBarChart *chart)
{
  if (!chart) return -1;
  return chart->hovered_bar;
}

ZuiWidget *zui_barchart_as_widget(ZuiBarChart *chart)
{
  return (ZuiWidget *)chart;
}

/* ========== LineChart ========== */

#define ZUI_LINECHART_MAX_POINTS 128

struct ZuiLineChart {
  ZuiWidget base;
  float values[ZUI_LINECHART_MAX_POINTS];
  size_t point_count;
  float max_value;
  ZuiColor line_color;
  float line_thickness;
  bool show_points;
};

static void linechart_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiLineChart *chart = (ZuiLineChart *)widget;
  if (chart->point_count < 2) return;

  float max_val = chart->max_value;
  if (max_val <= 0) {
    for (size_t i = 0; i < chart->point_count; i++) {
      if (chart->values[i] > max_val) max_val = chart->values[i];
    }
  }
  if (max_val <= 0) return;

  float step_x = widget->bounds.width / (float)(chart->point_count - 1);

  for (size_t i = 0; i < chart->point_count - 1; i++) {
    float x1 = widget->bounds.x + (float)i * step_x;
    float y1 = widget->bounds.y + widget->bounds.height -
               (chart->values[i] / max_val) * widget->bounds.height;
    float x2 = widget->bounds.x + (float)(i + 1) * step_x;
    float y2 = widget->bounds.y + widget->bounds.height -
               (chart->values[i + 1] / max_val) * widget->bounds.height;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f) continue;

    float angle = atan2f(dy, dx);
    float hw = chart->line_thickness / 2;

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float px1 = x1 - sin_a * hw;
    float py1 = y1 + cos_a * hw;

    zui_renderer_draw_rounded_rect(renderer,
      ZUI_RECT(px1 - hw, py1 - hw, len + chart->line_thickness, chart->line_thickness),
      chart->line_color, hw);
  }

  if (chart->show_points) {
    float point_radius = chart->line_thickness * 1.5f;
    for (size_t i = 0; i < chart->point_count; i++) {
      float x = widget->bounds.x + (float)i * step_x;
      float y = widget->bounds.y + widget->bounds.height -
                (chart->values[i] / max_val) * widget->bounds.height;
      zui_renderer_draw_circle(renderer, x, y, point_radius, chart->line_color);
    }
  }
}

static const ZuiWidgetVTable linechart_vtable = {
  .draw = linechart_draw,
};

ZuiLineChart *zui_linechart_create(void)
{
  ZuiLineChart *chart = (ZuiLineChart *)zui_widget_create(
    sizeof(ZuiLineChart), ZUI_WIDGET_LINE_CHART, &linechart_vtable);
  if (!chart) return NULL;

  chart->point_count = 0;
  chart->max_value = 0;
  chart->line_color = ZUI_COLOR_HEX(0x4a9eff);
  chart->line_thickness = 2.0f;
  chart->show_points = true;
  chart->base.preferred_size.width = 200.0f;
  chart->base.preferred_size.height = 100.0f;

  return chart;
}

void zui_linechart_set_size(ZuiLineChart *chart, float width, float height)
{
  if (chart) {
    chart->base.preferred_size.width = width;
    chart->base.preferred_size.height = height;
  }
}

void zui_linechart_add_point(ZuiLineChart *chart, float value)
{
  if (!chart || chart->point_count >= ZUI_LINECHART_MAX_POINTS) return;
  chart->values[chart->point_count++] = value;
}

void zui_linechart_clear(ZuiLineChart *chart)
{
  if (chart) chart->point_count = 0;
}

void zui_linechart_set_max_value(ZuiLineChart *chart, float max)
{
  if (chart) chart->max_value = max;
}

void zui_linechart_set_line_color(ZuiLineChart *chart, ZuiColor color)
{
  if (chart) chart->line_color = color;
}

void zui_linechart_set_line_thickness(ZuiLineChart *chart, float thickness)
{
  if (chart) chart->line_thickness = thickness;
}

void zui_linechart_set_show_points(ZuiLineChart *chart, bool show)
{
  if (chart) chart->show_points = show;
}

ZuiWidget *zui_linechart_as_widget(ZuiLineChart *chart)
{
  return (ZuiWidget *)chart;
}

/* ========== CircularProgress ========== */

struct ZuiCircularProgress {
  ZuiWidget base;
  float value;
  float thickness;
  ZuiColor track_color;
  ZuiColor fill_color;
  char *text;
  bool show_percentage;
  ZuiColor text_color;
  ZuiFont *font;
  float text_size;
  ZuiMouseButtonCallback on_click_cb;
  void *on_click_data;
};

static void circularprogress_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiCircularProgress *cp = (ZuiCircularProgress *)widget;

  float cx = widget->bounds.x + widget->bounds.width / 2;
  float cy = widget->bounds.y + widget->bounds.height / 2;
  float radius = (widget->bounds.width < widget->bounds.height ?
                  widget->bounds.width : widget->bounds.height) / 2 - 4;

  zui_renderer_draw_circle_outline(renderer, cx, cy, radius,
    cp->thickness, cp->track_color);

  if (cp->value > 0) {
    float start_angle = -ZUI_PI / 2;
    float end_angle = start_angle + cp->value * 2 * ZUI_PI;
    zui_renderer_draw_arc_outline(renderer, cx, cy, radius,
      start_angle, end_angle, cp->thickness, cp->fill_color);
  }

  if (cp->font && (cp->text || cp->show_percentage)) {
    char display_text[64];
    if (cp->text) {
      snprintf(display_text, sizeof(display_text), "%s", cp->text);
    } else {
      snprintf(display_text, sizeof(display_text), "%.0f%%",
               (double)(cp->value * 100));
    }

    float tw = zui_font_text_width(cp->font, display_text);
    float th = zui_font_text_height(cp->font, display_text);
    zui_font_render_text(cp->font, renderer,
      cx - tw / 2, cy - th / 2, display_text, cp->text_color);
  }
}

static void circularprogress_destroy(ZuiWidget *widget)
{
  ZuiCircularProgress *cp = (ZuiCircularProgress *)widget;
  free(cp->text);
  if (cp->font && cp->font != g_default_font) {
    zui_font_destroy(cp->font);
  }
}

static void circularprogress_on_mouse_down(ZuiWidget *widget, float x, float y,
                                            uint32_t button)
{
  (void)x;
  (void)y;
  ZuiCircularProgress *cp = (ZuiCircularProgress *)widget;
  if (cp->on_click_cb) {
    cp->on_click_cb(widget, button, cp->on_click_data);
  }
}

static const ZuiWidgetVTable circularprogress_vtable = {
  .draw = circularprogress_draw,
  .destroy = circularprogress_destroy,
  .on_mouse_down = circularprogress_on_mouse_down,
};

ZuiCircularProgress *zui_circularprogress_create(void)
{
  ZuiCircularProgress *cp = (ZuiCircularProgress *)zui_widget_create(
    sizeof(ZuiCircularProgress), ZUI_WIDGET_CIRCULAR_PROGRESS,
    &circularprogress_vtable);
  if (!cp) return NULL;

  cp->value = 0;
  cp->thickness = 8.0f;
  cp->track_color = ZUI_COLOR_HEX(0x333333);
  cp->fill_color = ZUI_COLOR_HEX(0x4a9eff);
  cp->text = NULL;
  cp->show_percentage = false;
  cp->text_color = ZUI_COLOR_HEX(0xffffff);
  cp->font = get_default_font();
  cp->base.preferred_size.width = 60.0f;
  cp->base.preferred_size.height = 60.0f;

  return cp;
}

void zui_circularprogress_set_size(ZuiCircularProgress *cp, float size)
{
  if (cp) {
    cp->base.preferred_size.width = size;
    cp->base.preferred_size.height = size;
  }
}

void zui_circularprogress_set_value(ZuiCircularProgress *cp, float value)
{
  if (cp) {
    cp->value = value < 0 ? 0 : (value > 1 ? 1 : value);
  }
}

float zui_circularprogress_get_value(ZuiCircularProgress *cp)
{
  return cp ? cp->value : 0;
}

void zui_circularprogress_set_thickness(ZuiCircularProgress *cp, float thickness)
{
  if (cp) cp->thickness = thickness;
}

void zui_circularprogress_set_colors(ZuiCircularProgress *cp, ZuiColor track,
                                      ZuiColor fill)
{
  if (cp) {
    cp->track_color = track;
    cp->fill_color = fill;
  }
}

void zui_circularprogress_set_text(ZuiCircularProgress *cp, const char *text)
{
  if (!cp) return;
  free(cp->text);
  cp->text = text ? strdup(text) : NULL;
}

void zui_circularprogress_set_show_percentage(ZuiCircularProgress *cp, bool show)
{
  if (cp) cp->show_percentage = show;
}

void zui_circularprogress_set_text_color(ZuiCircularProgress *cp, ZuiColor color)
{
  if (cp) cp->text_color = color;
}

void zui_circularprogress_set_text_size(ZuiCircularProgress *cp, float size)
{
  if (!cp) return;

  if (cp->font && fabsf(cp->font->size - size) < 0.5f) {
    return;
  }

  cp->text_size = size;

  if (cp->font && cp->font != g_default_font) {
    zui_font_destroy(cp->font);
    cp->font = NULL;
  }

  const char *font_paths[] = {
    "/usr/share/fonts/noto/NotoSans-Regular.ttf",
    "/usr/share/fonts/TTF/NotoSans-Regular.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans.ttf",
  };
  for (size_t i = 0; i < sizeof(font_paths) / sizeof(font_paths[0]); i++) {
    ZuiFont *font = zui_font_load(font_paths[i], size);
    if (font) {
      cp->font = font;
      break;
    }
  }
}

void zui_circularprogress_on_click(ZuiCircularProgress *cp,
                                    ZuiMouseButtonCallback callback,
                                    void *user_data)
{
  if (!cp) return;
  cp->on_click_cb = callback;
  cp->on_click_data = user_data;
}

ZuiWidget *zui_circularprogress_as_widget(ZuiCircularProgress *cp)
{
  return (ZuiWidget *)cp;
}
