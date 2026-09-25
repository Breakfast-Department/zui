#include <zui/internal/window_internal.h>
#include <zui/window.h>
#include <zui/internal/font_internal.h>
#include <zui/image.h>
#include <zui/font.h>
#include <zui/app.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <linux/limits.h>
#include <linux/input-event-codes.h>

#define ZUI_DEFAULT_MIN_WIDTH 64
#define ZUI_DEFAULT_MIN_HEIGHT 32
#define ZUI_RESIZE_BORDER 6.0f
#define ZUI_RESIZE_CORNER 12.0f
#define ZUI_HANDLER_INITIAL_CAPACITY 4

extern ZuiPlatform *zui_get_platform(void);
extern ZuiEglContext *zui_get_egl(void);
extern ZuiRenderer *zui_get_renderer(void);
extern bool zui_init_renderer_if_needed(void);

static ZuiWidget *g_hovered_widget = NULL;
static ZuiWidget *g_pressed_widget = NULL;
static uint32_t g_resize_edge = 0;

#define ZUI_DOUBLE_CLICK_TIME_MS 400
#define ZUI_DOUBLE_CLICK_DISTANCE 5

static struct timespec g_last_click_time = {0};
static double g_last_click_x = 0;
static double g_last_click_y = 0;
static ZuiWindow *g_last_click_window = NULL;

static long get_time_ms(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static bool is_double_click(ZuiWindow *window, double x, double y)
{
  long now = get_time_ms();
  long last = g_last_click_time.tv_sec * 1000 + g_last_click_time.tv_nsec / 1000000;
  long elapsed = now - last;

  double dx = x - g_last_click_x;
  double dy = y - g_last_click_y;
  double dist = dx * dx + dy * dy;

  bool is_dbl = (window == g_last_click_window &&
                 elapsed < ZUI_DOUBLE_CLICK_TIME_MS &&
                 dist < ZUI_DOUBLE_CLICK_DISTANCE * ZUI_DOUBLE_CLICK_DISTANCE);

  clock_gettime(CLOCK_MONOTONIC, &g_last_click_time);
  g_last_click_x = x;
  g_last_click_y = y;
  g_last_click_window = window;

  return is_dbl;
}

static bool overlay_hit_test(ZuiWidget *overlay, float x, float y);
void zui_window_maximize(ZuiWindow *window);

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

static void window_layout(ZuiWidget *widget);

static const ZuiWidgetVTable window_vtable = {
  .layout = window_layout,
};

static void container_layout(ZuiWidget *widget)
{
  float x = widget->bounds.x + widget->padding;
  float y = widget->bounds.y + widget->padding;
  float available_w = widget->bounds.width - widget->padding * 2;
  float available_h = widget->bounds.height - widget->padding * 2;

  if (widget->child_count == 0) return;

  for (int i = 0; i < widget->child_count; i++) {
    ZuiWidget *child = widget->children[i];
    if (widget->layout_dir == ZUI_LAYOUT_HORIZONTAL) {
      if (child->fill_height) child->preferred_size.height = available_h;
      if (child->fill_width) child->expand = true;
    } else {
      if (child->fill_width) child->preferred_size.width = available_w;
      if (child->fill_height) child->expand = true;
    }
  }

  float total_fixed = 0;
  int expand_count = 0;
  int visible_count = 0;

  for (int i = 0; i < widget->child_count; i++) {
    ZuiWidget *child = widget->children[i];
    if (!child->visible) continue;
    visible_count++;

    if (widget->layout_dir == ZUI_LAYOUT_HORIZONTAL) {
      if (child->expand) expand_count++;
      else total_fixed += child->preferred_size.width;
    } else {
      if (child->expand) expand_count++;
      else total_fixed += child->preferred_size.height;
    }
  }

  float total_spacing = (visible_count > 1) ? widget->spacing * (float)(visible_count - 1) : 0;
  float available = (widget->layout_dir == ZUI_LAYOUT_HORIZONTAL)
    ? available_w - total_fixed - total_spacing
    : available_h - total_fixed - total_spacing;

  float expand_size = (expand_count > 0 && available > 0) ? available / (float)expand_count : 0;

  if (widget->layout_dir == ZUI_LAYOUT_HORIZONTAL) {
    float total_content = total_fixed + total_spacing + (expand_count > 0 ? expand_size * expand_count : 0);
    float start_x = x;
    if (widget->align == ZUI_ALIGN_CENTER) {
      start_x = x + (available_w - total_content) / 2;
    } else if (widget->align == ZUI_ALIGN_END) {
      start_x = x + available_w - total_content;
    }

    float cx = start_x;
    for (int i = 0; i < widget->child_count; i++) {
      ZuiWidget *child = widget->children[i];
      if (!child->visible) continue;

      float cw = child->expand ? expand_size : child->preferred_size.width;
      float ch = child->preferred_size.height;
      float cy = y + (available_h - ch) / 2;

      zui_widget_set_bounds(child, cx, cy, cw, ch);
      cx += cw + widget->spacing;
    }
  } else {
    float total_content = total_fixed + total_spacing + (expand_count > 0 ? expand_size * expand_count : 0);
    float start_y = y;
    if (widget->align == ZUI_ALIGN_CENTER) {
      start_y = y + (available_h - total_content) / 2;
    } else if (widget->align == ZUI_ALIGN_END) {
      start_y = y + available_h - total_content;
    }

    float cy = start_y;
    for (int i = 0; i < widget->child_count; i++) {
      ZuiWidget *child = widget->children[i];
      if (!child->visible) continue;

      float cw = child->preferred_size.width;
      float ch = child->expand ? expand_size : child->preferred_size.height;

      zui_widget_set_bounds(child, x, cy, cw, ch);
      cy += ch + widget->spacing;
    }
  }
}

static const ZuiWidgetVTable container_vtable = {
  .layout = container_layout,
};

static void window_layout(ZuiWidget *widget)
{
  ZuiWindow *window = (ZuiWindow *)widget;
  float x = widget->bounds.x;
  float y = widget->bounds.y;
  float w = widget->bounds.width;
  float h = widget->bounds.height;

  if (window->content) {
    zui_widget_set_bounds(window->content, x, y, w, h);
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

static void init_handler_list(ZuiWidget ***list, int *count, int *capacity)
{
  *list = NULL;
  *count = 0;
  *capacity = 0;
}

static void free_handler_list(ZuiWidget **list)
{
  free(list);
}

static bool add_handler(ZuiWidget ***list, int *count, int *capacity, ZuiWidget *widget)
{
  for (int i = 0; i < *count; i++) {
    if ((*list)[i] == widget) return true;
  }

  if (*count >= *capacity) {
    int new_cap = *capacity == 0 ? ZUI_HANDLER_INITIAL_CAPACITY : *capacity * 2;
    ZuiWidget **new_list = realloc(*list, (size_t)new_cap * sizeof(ZuiWidget *));
    if (!new_list) return false;
    *list = new_list;
    *capacity = new_cap;
  }

  (*list)[*count] = widget;
  (*count)++;
  return true;
}

static void remove_handler(ZuiWidget **list, int *count, ZuiWidget *widget)
{
  for (int i = 0; i < *count; i++) {
    if (list[i] == widget) {
      for (int j = i; j < *count - 1; j++) {
        list[j] = list[j + 1];
      }
      (*count)--;
      return;
    }
  }
}

static bool is_widget_or_child_of(ZuiWidget *widget, ZuiWidget *target)
{
  if (widget == target) return true;
  ZuiWidget *parent = widget->parent;
  while (parent) {
    if (parent == target) return true;
    parent = parent->parent;
  }
  return false;
}

static bool is_moving_handler(ZuiWindow *window, ZuiWidget *widget)
{
  for (int i = 0; i < window->moving_handler_count; i++) {
    if (is_widget_or_child_of(widget, window->moving_handlers[i])) {
      return true;
    }
  }
  return false;
}

static bool is_interactive_widget(ZuiWidget *widget)
{
  return widget->type == ZUI_WIDGET_BUTTON ||
         widget->type == ZUI_WIDGET_TEXTINPUT ||
         widget->type == ZUI_WIDGET_CHECKBOX ||
         widget->type == ZUI_WIDGET_SLIDER ||
         widget->type == ZUI_WIDGET_DROPDOWN;
}

static bool is_decoration_widget(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window->decoration || !widget) return false;
  if (widget == window->decoration) return true;

  ZuiWidget *parent = widget->parent;
  while (parent) {
    if (parent == window->decoration) return true;
    parent = parent->parent;
  }
  return false;
}

ZuiWindow *zui_window_create(int width, int height, const char *title)
{
  ZuiPlatform *platform = zui_get_platform();
  ZuiEglContext *egl = zui_get_egl();

  ZuiWindow *window = (ZuiWindow *)zui_widget_create(
    sizeof(ZuiWindow), ZUI_WIDGET_WINDOW, &window_vtable);
  if (!window) return NULL;

  window->title = title ? strdup(title) : NULL;
  window->x = 0;
  window->y = 0;
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
  window->border_width = 0.5f;

  init_handler_list(&window->moving_handlers, &window->moving_handler_count,
                    &window->moving_handler_capacity);
  init_handler_list(&window->close_handlers, &window->close_handler_count,
                    &window->close_handler_capacity);
  init_handler_list(&window->maximize_handlers, &window->maximize_handler_count,
                    &window->maximize_handler_capacity);
  init_handler_list(&window->hide_handlers, &window->hide_handler_count,
                    &window->hide_handler_capacity);
  init_handler_list(&window->minimize_handlers, &window->minimize_handler_count,
                    &window->minimize_handler_capacity);

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

  window->content = create_container();
  if (window->content) {
    window->content->layout_dir = ZUI_LAYOUT_VERTICAL;
    window->content->align = ZUI_ALIGN_START;
    window->content->padding = 0.0f;
    window->content->spacing = 0.0f;
    zui_widget_add_child((ZuiWidget *)window, window->content);
  }

  window->overlay = NULL;

  return window;
}

void zui_window_destroy(ZuiWindow *window)
{
  if (!window) return;

  ZuiPlatform *platform = zui_get_platform();
  ZuiEglContext *egl = zui_get_egl();

  free_handler_list(window->moving_handlers);
  free_handler_list(window->close_handlers);
  free_handler_list(window->maximize_handlers);
  free_handler_list(window->hide_handlers);
  free_handler_list(window->minimize_handlers);

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

  bool no_frame = window->maximized || window->fullscreen;
  float radius = no_frame ? 0 : window->corner_radius;
  float border = no_frame ? 0 : window->border_width;
  float inset = border;
  float inner_radius = radius > inset ? radius - inset : 0;

  if (zui_window_is_maximized(window)) {
    window->content->corner_radius = 0;
  } else {
    window->content->corner_radius = window->corner_radius - window->border_width;
  }

  zui_widget_set_bounds((ZuiWidget *)window, inset, inset,
                         (float)window->width - inset * 2,
                         (float)window->height - inset * 2);
  zui_widget_layout((ZuiWidget *)window);

  zui_renderer_begin(renderer, window->width, window->height);
  zui_renderer_clear(renderer, ZUI_COLOR(0, 0, 0, 0));

  if (border > 0 && !no_frame) {
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

  if (window->custom_draw) {
    window->custom_draw(window, window->custom_draw_user_data);
  }

  if (window->overlay && window->overlay->vtable &&
      window->overlay->vtable->draw_overlay) {
    window->overlay->vtable->draw_overlay(window->overlay, renderer);
  }

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
  window->content->corner_radius = radius - 0.5f;
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

  if (g_pressed_widget && g_pressed_widget->vtable &&
      g_pressed_widget->vtable->on_mouse_move) {
    g_pressed_widget->vtable->on_mouse_move(g_pressed_widget, (float)x, (float)y);
    window->needs_redraw = true;
  }

  if (window->overlay && window->overlay->vtable &&
      window->overlay->vtable->on_mouse_move) {
    window->overlay->vtable->on_mouse_move(window->overlay, (float)x, (float)y);
  }

  g_resize_edge = detect_resize_edge(window, (float)x, (float)y);

  if (g_resize_edge) {
    update_hover(window, NULL);
    ZuiPlatform *platform = zui_get_platform();
    zui_platform_set_cursor(platform, resize_edge_to_cursor(g_resize_edge));
  } else {
    ZuiWidget *hit = NULL;
    if (window->overlay && overlay_hit_test(window->overlay, (float)x, (float)y)) {
      hit = window->overlay;
    } else {
      hit = zui_widget_hit_test((ZuiWidget *)window, (float)x, (float)y);
    }
    update_hover(window, hit);
  }
}

static bool overlay_hit_test(ZuiWidget *overlay, float x, float y)
{
  if (!overlay || !overlay->vtable || !overlay->vtable->hit_test) {
    return false;
  }
  return overlay->vtable->hit_test(overlay, x, y);
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

  ZuiWidget *hit = NULL;

  if (window->overlay && overlay_hit_test(window->overlay, (float)x, (float)y)) {
    hit = window->overlay;
  } else {
    hit = zui_widget_hit_test((ZuiWidget *)window, (float)x, (float)y);
  }

  if (pressed && (button == BTN_LEFT || button == BTN_RIGHT)) {
    if (button == BTN_LEFT) {
      g_pressed_widget = hit;
    }

    if (button == BTN_RIGHT && is_decoration_widget(window, hit)) {
      ZuiPlatform *platform = zui_get_platform();
      zui_wayland_show_window_menu(platform, &window->wayland, (int)x, (int)y);
      return;
    }

    if (hit && hit->vtable && hit->vtable->on_mouse_down) {
      hit->vtable->on_mouse_down(hit, (float)x, (float)y, button);
    }

    if (hit && button == BTN_LEFT) {
      hit->pressed = true;
      window->needs_redraw = true;
    }

    if (hit && button == BTN_LEFT && is_moving_handler(window, hit)) {
      bool on_interactive = is_interactive_widget(hit);
      if (!on_interactive) {
        ZuiWidget *p = hit->parent;
        while (p) {
          if (is_interactive_widget(p)) {
            on_interactive = true;
            break;
          }
          p = p->parent;
        }
      }

      if (!on_interactive) {
        if (is_double_click(window, x, y)) {
          zui_window_maximize(window);
        } else {
          ZuiPlatform *platform = zui_get_platform();
          zui_wayland_start_move(platform, &window->wayland);
        }
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

void zui_window_handle_key(ZuiWindow *window, uint32_t key, uint32_t sym,
                            const char *text, bool pressed)
{
  if (!window) return;

  ZuiWidget *focused = window->focused;
  if (focused && focused->vtable && focused->vtable->on_key) {
    focused->vtable->on_key(focused, key, sym, text, pressed);
    window->needs_redraw = true;
  }
}

void zui_window_handle_scroll(ZuiWindow *window, double x, double y,
                               double dx, double dy)
{
  if (!window) return;

  ZuiWidget *hit = zui_widget_hit_test((ZuiWidget *)window, (float)x, (float)y);

  while (hit) {
    if (hit->vtable && hit->vtable->on_scroll) {
      hit->vtable->on_scroll(hit, dx, dy);
      window->needs_redraw = true;
      return;
    }
    hit = hit->parent;
  }
}

void zui_window_request_close(ZuiWindow *window)
{
  if (window) window->running = false;
}

void zui_window_set_maximized_state(ZuiWindow *window, bool maximized)
{
  if (window) {
    window->maximized = maximized;
    window->needs_redraw = true;
  }
}

void zui_window_set_fullscreen_state(ZuiWindow *window, bool fullscreen)
{
  if (!window) return;

  bool was_fullscreen = window->fullscreen;
  window->fullscreen = fullscreen;

  if (fullscreen != was_fullscreen) {
    window->base.corner_mode = fullscreen ? ZUI_CORNERS_NONE : ZUI_CORNERS_ALL;

    if (window->decoration) {
      window->decoration->visible = !fullscreen && window->decorated;
    }

    window->needs_redraw = true;
  }
}

void zui_window_set_decorated_state(ZuiWindow *window, bool decorated)
{
  if (!window) return;
  window->decorated = decorated;

  if (window->decoration) {
    window->decoration->visible = decorated;
  }

  window->needs_redraw = true;
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

void zui_window_set_overlay(ZuiWindow *window, ZuiWidget *widget)
{
  if (window) window->overlay = widget;
}

void zui_window_clear_overlay(ZuiWindow *window, ZuiWidget *widget)
{
  if (window && window->overlay == widget) window->overlay = NULL;
}

void zui_window_minimize(ZuiWindow *window)
{
  if (!window) return;
  window->minimized = true;
  zui_wayland_minimize(&window->wayland);
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

void zui_window_hide(ZuiWindow *window)
{
  if (!window) return;
  window->hidden = true;
  zui_wayland_minimize(&window->wayland);
}

void zui_window_close(ZuiWindow *window)
{
  if (window) window->running = false;
}

bool zui_window_is_maximized(ZuiWindow *window)
{
  return window ? window->maximized : false;
}

bool zui_window_is_minimized(ZuiWindow *window)
{
  return window ? window->minimized : false;
}

bool zui_window_is_hidden(ZuiWindow *window)
{
  return window ? window->hidden : false;
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

void zui_window_get_size(ZuiWindow *window, int *width, int *height)
{
  if (!window) return;
  if (width) *width = window->width;
  if (height) *height = window->height;
}

void zui_window_set_position(ZuiWindow *window, int x, int y)
{
  if (!window) return;
  window->x = x;
  window->y = y;
}

void zui_window_get_position(ZuiWindow *window, int *x, int *y)
{
  if (!window) return;
  if (x) *x = window->x;
  if (y) *y = window->y;
}

bool zui_window_is_active(ZuiWindow *window)
{
  return window ? window->active : false;
}

void zui_window_show_menu(ZuiWindow *window, int x, int y)
{
  if (!window) return;
  ZuiPlatform *platform = zui_get_platform();
  zui_wayland_show_window_menu(platform, &window->wayland, x, y);
}

void zui_window_set_fullscreen(ZuiWindow *window, bool fullscreen)
{
  if (!window) return;
  window->fullscreen = fullscreen;
  zui_wayland_set_fullscreen(&window->wayland, fullscreen);
}

bool zui_window_is_fullscreen(ZuiWindow *window)
{
  return window ? window->fullscreen : false;
}

void zui_window_set_decorated(ZuiWindow *window, bool decorated)
{
  if (!window) return;
  window->decorated = decorated;
  ZuiPlatform *platform = zui_get_platform();
  zui_wayland_set_decorated(platform, &window->wayland, decorated);
}

bool zui_window_is_decorated(ZuiWindow *window)
{
  return window ? window->decorated : true;
}

void zui_add_window_moving_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  add_handler(&window->moving_handlers, &window->moving_handler_count,
              &window->moving_handler_capacity, widget);
}

void zui_remove_window_moving_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  remove_handler(window->moving_handlers, &window->moving_handler_count, widget);
}

void zui_add_close_window_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  add_handler(&window->close_handlers, &window->close_handler_count,
              &window->close_handler_capacity, widget);
}

void zui_remove_close_window_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  remove_handler(window->close_handlers, &window->close_handler_count, widget);
}

void zui_add_window_maximize_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  add_handler(&window->maximize_handlers, &window->maximize_handler_count,
              &window->maximize_handler_capacity, widget);
}

void zui_remove_window_maximize_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  remove_handler(window->maximize_handlers, &window->maximize_handler_count, widget);
}

void zui_add_hide_window_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  add_handler(&window->hide_handlers, &window->hide_handler_count,
              &window->hide_handler_capacity, widget);
}

void zui_remove_hide_window_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  remove_handler(window->hide_handlers, &window->hide_handler_count, widget);
}

void zui_add_minimize_window_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  add_handler(&window->minimize_handlers, &window->minimize_handler_count,
              &window->minimize_handler_capacity, widget);
}

void zui_remove_minimize_window_handler(ZuiWindow *window, ZuiWidget *widget)
{
  if (!window || !widget) return;
  remove_handler(window->minimize_handlers, &window->minimize_handler_count, widget);
}

void zui_set_window_decoration(ZuiWindow *window, ZuiWidget *decoration)
{
  if (!window) return;
  window->decoration = decoration;
}

ZuiWidget *zui_get_window_decoration(ZuiWindow *window)
{
  return window ? window->decoration : NULL;
}

void zui_window_decoration_set_visible(ZuiWindow *window, bool visible)
{
  if (!window || !window->decoration) return;
  window->decoration->visible = visible;
  window->needs_redraw = true;
}

bool zui_window_decoration_is_visible(ZuiWindow *window)
{
  if (!window || !window->decoration) return false;
  return window->decoration->visible;
}

void zui_window_set_custom_draw(ZuiWindow *window,
                                 void (*callback)(ZuiWindow *window, void *user_data),
                                 void *user_data)
{
  if (!window) return;
  window->custom_draw = callback;
  window->custom_draw_user_data = user_data;
}
