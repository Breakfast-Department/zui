# API Reference

Complete reference for the ZUI public API.

## Initialization

```c
// Initialize ZUI
bool zui_init(void);

// Shutdown ZUI
void zui_shutdown(void);

// Poll events (call in main loop)
void zui_poll_events(void);
```

## Window

### Creation & Destruction

```c
ZuiWindow *zui_window_create(int width, int height, const char *title);
void zui_window_destroy(ZuiWindow *window);
```

### Window State

```c
void zui_window_show(ZuiWindow *window);
bool zui_window_running(ZuiWindow *window);
void zui_window_close(ZuiWindow *window);
void zui_window_render(ZuiWindow *window);
```

### Size & Position

```c
void zui_window_get_size(ZuiWindow *window, int *width, int *height);
void zui_window_set_min_size(ZuiWindow *window, int width, int height);
void zui_window_set_max_size(ZuiWindow *window, int width, int height);
```

### Appearance

```c
void zui_window_set_background_color(ZuiWindow *window, ZuiColor color);
void zui_window_set_corner_radius(ZuiWindow *window, float radius);
void zui_window_set_title(ZuiWindow *window, const char *title);
```

### Content

```c
ZuiWidget *zui_window_content(ZuiWindow *window);
ZuiTitlebar *zui_window_titlebar(ZuiWindow *window);
ZuiWidget *zui_window_close_button(ZuiWindow *window);
```

---

## Widget (Base)

### Type System

```c
ZuiWidgetType zui_widget_get_type(ZuiWidget *widget);
```

### Type Macros

```c
// Casting
#define ZUI_WIDGET(obj)  ((ZuiWidget *)(obj))

// Type checking
#define ZUI_IS_BUTTON(w)
#define ZUI_IS_LABEL(w)
#define ZUI_IS_PANEL(w)
#define ZUI_IS_CHECKBOX(w)
#define ZUI_IS_TEXTINPUT(w)
#define ZUI_IS_SLIDER(w)
#define ZUI_IS_DROPDOWN(w)
#define ZUI_IS_SCROLLVIEW(w)
#define ZUI_IS_SCROLLER(w)
#define ZUI_IS_SPLITVIEW(w)
#define ZUI_IS_PROGRESSBAR(w)
#define ZUI_IS_GRIDVIEW(w)
#define ZUI_IS_CIRCULAR_PROGRESS(w)
#define ZUI_IS_PIECHART(w)
#define ZUI_IS_BARCHART(w)
#define ZUI_IS_LINECHART(w)
#define ZUI_IS_IMAGE(w)
#define ZUI_IS_ICON(w)

// Safe downcasting (returns NULL if type doesn't match)
#define ZUI_BUTTON(w)
#define ZUI_LABEL(w)
#define ZUI_PANEL(w)
// ... etc for each widget type
```

### Hierarchy

```c
void zui_widget_add_child(ZuiWidget *parent, ZuiWidget *child);
void zui_widget_remove_child(ZuiWidget *parent, ZuiWidget *child);
ZuiWidget *zui_widget_get_parent(ZuiWidget *widget);
int zui_widget_get_child_count(ZuiWidget *widget);
ZuiWidget *zui_widget_get_child(ZuiWidget *widget, int index);
```

### Visibility

```c
void zui_widget_set_visible(ZuiWidget *widget, bool visible);
bool zui_widget_is_visible(ZuiWidget *widget);
```

### Styling

```c
void zui_widget_set_background(ZuiWidget *widget, ZuiColor color);
void zui_widget_set_corner_radius(ZuiWidget *widget, float radius);
void zui_widget_set_cursor(ZuiWidget *widget, ZuiCursor cursor);
// Alias:
void zui_set_cursor(ZuiWidget *widget, ZuiCursor cursor);
```

### Sizing

```c
void zui_widget_set_size(ZuiWidget *widget, float width, float height);
void zui_widget_set_fill(ZuiWidget *widget, bool fill_width, bool fill_height);
void zui_widget_set_padding(ZuiWidget *widget, float padding);
void zui_widget_set_spacing(ZuiWidget *widget, float spacing);
```

### Layout

```c
void zui_widget_set_layout(ZuiWidget *widget, ZuiLayoutDir direction);
void zui_widget_set_alignment(ZuiWidget *widget, ZuiAlign main, ZuiAlign cross);
```

### Focus

```c
void zui_widget_focus(ZuiWidget *widget);
void zui_widget_unfocus(ZuiWidget *widget);
```

### Destruction

```c
void zui_widget_destroy(ZuiWidget *widget);
```

---

## Button

### Creation

```c
ZuiWidget *zui_button_new(const char *text);
ZuiButton *zui_button_create(const char *text);
```

### Configuration

```c
void zui_button_set_text(ZuiButton *button, const char *text);
const char *zui_button_get_text(ZuiButton *button);
void zui_button_set_colors(ZuiButton *button, ZuiColor normal, ZuiColor hover, ZuiColor pressed);
void zui_button_set_text_color(ZuiButton *button, ZuiColor color);
void zui_button_set_corner_radius(ZuiButton *button, float radius);
void zui_button_set_icon(ZuiButton *button, ZuiIconSource *icon, float size);
void zui_button_set_icon_spacing(ZuiButton *button, float spacing);
```

### Events

```c
typedef void (*ZuiClickCallback)(ZuiWidget *widget, void *data);
void zui_button_on_click(ZuiButton *button, ZuiClickCallback callback, void *data);
```

### Conversion

```c
ZuiWidget *zui_button_as_widget(ZuiButton *button);
```

---

## Label

### Creation

```c
ZuiLabel *zui_label_create(const char *text);
```

### Configuration

```c
void zui_label_set_text(ZuiLabel *label, const char *text);
const char *zui_label_get_text(ZuiLabel *label);
void zui_label_set_color(ZuiLabel *label, ZuiColor color);
void zui_label_set_size(ZuiLabel *label, float size);
void zui_label_set_font(ZuiLabel *label, ZuiFont *font);
```

### Conversion

```c
ZuiWidget *zui_label_as_widget(ZuiLabel *label);
```

---

## Panel

### Creation

```c
ZuiPanel *zui_panel_create(void);
```

### Layout

```c
void zui_panel_set_layout(ZuiPanel *panel, ZuiLayoutDir direction);
void zui_panel_set_spacing(ZuiPanel *panel, float spacing);
void zui_panel_set_padding(ZuiPanel *panel, float top, float right, float bottom, float left);
void zui_panel_set_alignment(ZuiPanel *panel, ZuiAlign main, ZuiAlign cross);
```

### Sizing

```c
void zui_panel_set_size(ZuiPanel *panel, float width, float height);
void zui_panel_set_fill(ZuiPanel *panel, bool fill_width, bool fill_height);
```

### Styling

```c
void zui_panel_set_background(ZuiPanel *panel, ZuiColor color);
void zui_panel_set_corner_radius(ZuiPanel *panel, float radius);
```

### Children

```c
void zui_panel_add_child(ZuiPanel *panel, ZuiWidget *child);
void zui_panel_remove_child(ZuiPanel *panel, ZuiWidget *child);
void zui_panel_clear(ZuiPanel *panel);
```

### Conversion

```c
ZuiWidget *zui_panel_as_widget(ZuiPanel *panel);
```

---

## TextInput

### Creation

```c
ZuiTextInput *zui_textinput_create(const char *placeholder);
```

### Text

```c
void zui_textinput_set_text(ZuiTextInput *input, const char *text);
const char *zui_textinput_get_text(ZuiTextInput *input);
void zui_textinput_set_placeholder(ZuiTextInput *input, const char *text);
```

### Styling

```c
void zui_textinput_set_bg_color(ZuiTextInput *input, ZuiColor color);
void zui_textinput_set_text_color(ZuiTextInput *input, ZuiColor color);
void zui_textinput_set_placeholder_color(ZuiTextInput *input, ZuiColor color);
void zui_textinput_set_border_color(ZuiTextInput *input, ZuiColor color);
void zui_textinput_set_selection_color(ZuiTextInput *input, ZuiColor color);
void zui_textinput_set_corner_radius(ZuiTextInput *input, float radius);
void zui_textinput_set_padding(ZuiTextInput *input, float padding);
```

### Events

```c
typedef void (*ZuiTextInputCallback)(ZuiTextInput *input, const char *text, void *data);
void zui_textinput_on_change(ZuiTextInput *input, ZuiTextInputCallback callback, void *data);
```

---

## Slider

### Creation

```c
ZuiSlider *zui_slider_create(float min, float max, float value);
```

### Value

```c
void zui_slider_set_value(ZuiSlider *slider, float value);
float zui_slider_get_value(ZuiSlider *slider);
```

### Styling

```c
void zui_slider_set_track_color(ZuiSlider *slider, ZuiColor color);
void zui_slider_set_fill_color(ZuiSlider *slider, ZuiColor color);
void zui_slider_set_thumb_colors(ZuiSlider *slider, ZuiColor normal, ZuiColor hover, ZuiColor drag);
void zui_slider_set_thumb_radius(ZuiSlider *slider, float radius);
```

### Events

```c
typedef void (*ZuiSliderCallback)(ZuiSlider *slider, float value, void *data);
void zui_slider_on_change(ZuiSlider *slider, ZuiSliderCallback callback, void *data);
```

---

## Checkbox

### Creation

```c
ZuiCheckbox *zui_checkbox_create(const char *label);
```

### State

```c
void zui_checkbox_set_checked(ZuiCheckbox *cb, bool checked);
bool zui_checkbox_is_checked(ZuiCheckbox *cb);
```

### Events

```c
typedef void (*ZuiCheckboxCallback)(ZuiCheckbox *cb, bool checked, void *data);
void zui_checkbox_on_change(ZuiCheckbox *cb, ZuiCheckboxCallback callback, void *data);
```

---

## Dropdown

### Creation

```c
ZuiDropdown *zui_dropdown_create(const char *placeholder);
```

### Items

```c
void zui_dropdown_add_item(ZuiDropdown *dd, const char *item);
void zui_dropdown_clear_items(ZuiDropdown *dd);
int zui_dropdown_get_selected_index(ZuiDropdown *dd);
const char *zui_dropdown_get_selected_item(ZuiDropdown *dd);
```

### Events

```c
typedef void (*ZuiDropdownCallback)(ZuiDropdown *dd, int index, const char *item, void *data);
void zui_dropdown_on_change(ZuiDropdown *dd, ZuiDropdownCallback callback, void *data);
```

---

## ProgressBar

### Creation

```c
ZuiProgressBar *zui_progressbar_create(void);
```

### Value

```c
void zui_progressbar_set_value(ZuiProgressBar *bar, float value);  // 0.0 - 1.0
float zui_progressbar_get_value(ZuiProgressBar *bar);
```

### Styling

```c
void zui_progressbar_set_colors(ZuiProgressBar *bar, ZuiColor track, ZuiColor fill);
void zui_progressbar_set_corner_radius(ZuiProgressBar *bar, float radius);
```

---

## CircularProgress

### Creation

```c
ZuiCircularProgress *zui_circularprogress_create(void);
```

### Value

```c
void zui_circularprogress_set_value(ZuiCircularProgress *cp, float value);
```

### Styling

```c
void zui_circularprogress_set_size(ZuiCircularProgress *cp, float size);
void zui_circularprogress_set_thickness(ZuiCircularProgress *cp, float thickness);
void zui_circularprogress_set_colors(ZuiCircularProgress *cp, ZuiColor track, ZuiColor fill);
```

### Text

```c
void zui_circularprogress_set_text(ZuiCircularProgress *cp, const char *text);
void zui_circularprogress_set_text_color(ZuiCircularProgress *cp, ZuiColor color);
void zui_circularprogress_set_text_size(ZuiCircularProgress *cp, float size);
```

### Events

```c
void zui_circularprogress_on_click(ZuiCircularProgress *cp, 
    void (*callback)(ZuiWidget *, uint32_t, void *), void *data);
```

---

## ScrollView

### Creation

```c
ZuiScrollView *zui_scrollview_create(void);
```

### Content

```c
void zui_scrollview_set_content(ZuiScrollView *sv, ZuiWidget *content);
```

### Scrolling

```c
void zui_scrollview_scroll_to(ZuiScrollView *sv, float x, float y);
void zui_scrollview_get_scroll(ZuiScrollView *sv, float *x, float *y);
```

---

## GridView

### Creation

```c
ZuiGridView *zui_gridview_create(int columns);
```

### Configuration

```c
void zui_gridview_set_columns(ZuiGridView *gv, int columns);
void zui_gridview_set_gap(ZuiGridView *gv, float gap_x, float gap_y);
void zui_gridview_set_padding(ZuiGridView *gv, float padding);
void zui_gridview_set_cell_size(ZuiGridView *gv, float width, float height);
```

### Children

```c
void zui_gridview_add_child(ZuiGridView *gv, ZuiWidget *child);
void zui_gridview_remove_child(ZuiGridView *gv, ZuiWidget *child);
void zui_gridview_clear(ZuiGridView *gv);
```

---

## Image

### Creation

```c
ZuiImage *zui_image_create(const char *path);
ZuiImage *zui_image_create_from_memory(const unsigned char *data, int len);
```

### Configuration

```c
void zui_image_set_size(ZuiImage *image, float width, float height);
void zui_image_set_visible(ZuiImage *image, bool visible);
```

---

## Icon

### Creation

```c
ZuiIcon *zui_icon_create(const char *path);
```

### Configuration

```c
void zui_icon_set_size(ZuiIcon *icon, float size);
void zui_icon_set_color(ZuiIcon *icon, ZuiColor color);
```

---

## Font

```c
ZuiFont *zui_font_load(const char *path, float size);
ZuiFont *zui_font_default(void);
void zui_font_destroy(ZuiFont *font);
```

---

## Audio

### Initialization

```c
bool zui_audio_init(void);
void zui_audio_shutdown(void);
```

### Loading & Playback

```c
ZuiAudio *zui_audio_load(const char *path);
void zui_audio_destroy(ZuiAudio *audio);
void zui_audio_play(ZuiAudio *audio);
void zui_audio_stop(ZuiAudio *audio);
void zui_audio_set_volume(ZuiAudio *audio, float volume);
```

---

## Path

Paths allow drawing custom shapes with lines, curves, and arcs.

### Creation

```c
ZuiPath *zui_path_create(const ZuiPathCommand *commands, int count);
void zui_path_destroy(ZuiPath *path);
```

### Path Commands

```c
void zui_path_move_to(ZuiPath *path, float x, float y);
void zui_path_line_to(ZuiPath *path, float x, float y);
void zui_path_quad_to(ZuiPath *path, float cx, float cy, float x, float y);
void zui_path_cubic_to(ZuiPath *path, float c1x, float c1y, float c2x, float c2y, float x, float y);
void zui_path_arc_to(ZuiPath *path, float cx, float cy, float radius, float start_angle, float end_angle);
void zui_path_close(ZuiPath *path);
```

### Helper Shapes

```c
ZuiPath *zui_path_regular_polygon(float cx, float cy, float radius, int sides);
ZuiPath *zui_path_rounded_polygon(float *points, int count, float radius);
ZuiPath *zui_path_star(float cx, float cy, float outer_radius, float inner_radius, int points);
```

### Drawing

```c
void zui_draw_path(ZuiPath *path, ZuiColor fill_color);
void zui_draw_path_stroke(ZuiPath *path, ZuiColor stroke_color, float thickness);
```

### Utilities

```c
void zui_path_get_bounds(ZuiPath *path, float *x, float *y, float *w, float *h);
bool zui_path_contains(ZuiPath *path, float x, float y);
void zui_path_translate(ZuiPath *path, float dx, float dy);
void zui_path_scale(ZuiPath *path, float sx, float sy);
```

---

## Shape Widget

Shape widgets render custom paths as interactive widgets with support for children, events, and clipping.

### Creation

```c
void zui_shape_destroy(ZuiWidget *widget);
```

### Configuration

```c
void zui_shape_set_path(ZuiShape *shape, ZuiPath *path);
ZuiPath *zui_shape_get_path(ZuiShape *shape);
void zui_shape_set_fill(ZuiShape *shape, ZuiColor color);
void zui_shape_set_stroke(ZuiShape *shape, ZuiColor color, float thickness);
void zui_shape_set_cursor(ZuiShape *shape, ZuiCursor cursor);
```

### Children

```c
void zui_shape_add_child(ZuiShape *shape, ZuiWidget *child);
void zui_shape_remove_child(ZuiShape *shape, ZuiWidget *child);
```

### Events

```c
typedef void (*ZuiShapeClickCallback)(ZuiShape *shape, void *user_data);
typedef void (*ZuiShapeHoverCallback)(ZuiShape *shape, bool entered, void *user_data);

void zui_shape_on_click(ZuiShape *shape, ZuiShapeClickCallback callback, void *user_data);
void zui_shape_on_hover(ZuiShape *shape, ZuiShapeHoverCallback callback, void *user_data);
```

---

## Enums

### ZuiWidgetType

```c
typedef enum {
    ZUI_WIDGET_WINDOW,
    ZUI_WIDGET_CONTAINER,
    ZUI_WIDGET_BUTTON,
    ZUI_WIDGET_LABEL,
    ZUI_WIDGET_PANEL,
    ZUI_WIDGET_CHECKBOX,
    ZUI_WIDGET_TEXTINPUT,
    ZUI_WIDGET_SLIDER,
    ZUI_WIDGET_DROPDOWN,
    ZUI_WIDGET_SCROLLVIEW,
    ZUI_WIDGET_SCROLLER,
    ZUI_WIDGET_SPLITVIEW,
    ZUI_WIDGET_PROGRESSBAR,
    ZUI_WIDGET_GRIDVIEW,
    ZUI_WIDGET_MENUBAR,
    ZUI_WIDGET_MENU,
    ZUI_WIDGET_MENUITEM,
    ZUI_WIDGET_PIE_CHART,
    ZUI_WIDGET_BAR_CHART,
    ZUI_WIDGET_LINE_CHART,
    ZUI_WIDGET_CIRCULAR_PROGRESS,
    ZUI_WIDGET_RADIOBUTTON,
    ZUI_WIDGET_ICON,
    ZUI_WIDGET_IMAGE,
    ZUI_WIDGET_VIDEO,
    ZUI_WIDGET_TABVIEW,
    ZUI_WIDGET_TREEVIEW,
    ZUI_WIDGET_TEXTAREA,
    // ...
} ZuiWidgetType;
```

### ZuiLayoutDir

```c
typedef enum {
    ZUI_LAYOUT_HORIZONTAL,
    ZUI_LAYOUT_VERTICAL,
} ZuiLayoutDir;
```

### ZuiAlign

```c
typedef enum {
    ZUI_ALIGN_START,
    ZUI_ALIGN_CENTER,
    ZUI_ALIGN_END,
} ZuiAlign;
```

### ZuiCursor

```c
typedef enum {
    ZUI_CURSOR_DEFAULT,
    ZUI_CURSOR_POINTER,
    ZUI_CURSOR_TEXT,
    ZUI_CURSOR_CROSSHAIR,
    ZUI_CURSOR_MOVE,
    ZUI_CURSOR_RESIZE_EW,
    ZUI_CURSOR_RESIZE_NS,
    ZUI_CURSOR_RESIZE_NWSE,
    ZUI_CURSOR_RESIZE_NESW,
} ZuiCursor;
```

---

## Color Macros

```c
// RGBA (0.0 - 1.0)
#define ZUI_COLOR(r, g, b, a)

// RGB (alpha = 1.0)
#define ZUI_COLOR_RGB(r, g, b)

// Hex color (0xRRGGBB)
#define ZUI_COLOR_HEX(hex)
```

---

## Rect Macros

```c
#define ZUI_RECT(x, y, w, h)
```
