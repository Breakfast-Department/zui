#ifndef ZUI_WIDGET_H
#define ZUI_WIDGET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "color.h"
#include "cursor.h"
#include "font.h"
#include "layout.h"

typedef struct ZuiWidget ZuiWidget;
typedef struct ZuiButton ZuiButton;
typedef struct ZuiText ZuiText;
typedef struct ZuiPanel ZuiPanel;
typedef struct ZuiCheckbox ZuiCheckbox;
typedef struct ZuiScrollView ZuiScrollView;
typedef struct ZuiScroller ZuiScroller;
typedef struct ZuiSplitView ZuiSplitView;
typedef struct ZuiRadioGroup ZuiRadioGroup;
typedef struct ZuiRadioButton ZuiRadioButton;
typedef struct ZuiTextInput ZuiTextInput;
typedef struct ZuiSlider ZuiSlider;
typedef struct ZuiDropdown ZuiDropdown;
typedef struct ZuiProgressBar ZuiProgressBar;
typedef struct ZuiGridView ZuiGridView;
typedef struct ZuiMenuBar ZuiMenuBar;
typedef struct ZuiMenu ZuiMenu;
typedef struct ZuiMenuItem ZuiMenuItem;
typedef struct ZuiPieChart ZuiPieChart;
typedef struct ZuiBarChart ZuiBarChart;
typedef struct ZuiShape ZuiShape;
typedef struct ZuiLineChart ZuiLineChart;
typedef struct ZuiCircularProgress ZuiCircularProgress;

typedef void (*ZuiMenuItemCallback)(ZuiMenuItem *item, void *user_data);

typedef void (*ZuiCheckboxCallback)(ZuiCheckbox *checkbox, bool checked,
                                     void *user_data);
typedef void (*ZuiScrollerCallback)(ZuiScroller *scroller, float value,
                                     void *user_data);
typedef void (*ZuiRadioCallback)(ZuiRadioGroup *group, ZuiRadioButton *selected,
                                  void *user_data);
typedef void (*ZuiTextInputCallback)(ZuiTextInput *input, const char *text,
                                      void *user_data);
typedef void (*ZuiSliderCallback)(ZuiSlider *slider, float value,
                                   void *user_data);
typedef void (*ZuiDropdownCallback)(ZuiDropdown *dropdown, int index,
                                     const char *item, void *user_data);

typedef void (*ZuiClickCallback)(ZuiWidget *widget, void *user_data);
typedef void (*ZuiMouseButtonCallback)(ZuiWidget *widget, uint32_t button,
                                        void *user_data);

#define BTN_LEFT 0x110
#define BTN_RIGHT 0x111


typedef enum ZuiWidgetType {
  ZUI_WIDGET_CONTAINER,
  ZUI_WIDGET_WINDOW,
  ZUI_WIDGET_TITLEBAR,
  ZUI_WIDGET_BUTTON,
  ZUI_WIDGET_TEXT,
  ZUI_WIDGET_ICON,
  ZUI_WIDGET_IMAGE,
  ZUI_WIDGET_PANEL,
  ZUI_WIDGET_CHECKBOX,
  ZUI_WIDGET_SCROLLVIEW,
  ZUI_WIDGET_SCROLLER,
  ZUI_WIDGET_SPLITVIEW,
  ZUI_WIDGET_RADIOBUTTON,
  ZUI_WIDGET_TEXTINPUT,
  ZUI_WIDGET_SLIDER,
  ZUI_WIDGET_DROPDOWN,
  ZUI_WIDGET_PROGRESSBAR,
  ZUI_WIDGET_GRIDVIEW,
  ZUI_WIDGET_MENUBAR,
  ZUI_WIDGET_MENU,
  ZUI_WIDGET_MENUITEM,
  ZUI_WIDGET_PIE_CHART,
  ZUI_WIDGET_BAR_CHART,
  ZUI_WIDGET_LINE_CHART,
  ZUI_WIDGET_CIRCULAR_PROGRESS,
  ZUI_WIDGET_VIDEO,
  ZUI_WIDGET_TABVIEW,
  ZUI_WIDGET_TREEVIEW,
  ZUI_WIDGET_TREENODE,
  ZUI_WIDGET_TEXTAREA,
  ZUI_WIDGET_SHAPE,
} ZuiWidgetType;

ZuiWidgetType zui_widget_get_type(ZuiWidget *widget);

#define ZUI_WIDGET(obj) ((ZuiWidget *)(obj))

#define ZUI_IS_BUTTON(w)     ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_BUTTON)
#define ZUI_IS_TEXT(w)        ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_TEXT)
#define ZUI_IS_PANEL(w)      ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_PANEL)
#define ZUI_IS_CHECKBOX(w)   ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_CHECKBOX)
#define ZUI_IS_TEXTINPUT(w)  ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_TEXTINPUT)
#define ZUI_IS_SLIDER(w)     ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_SLIDER)
#define ZUI_IS_DROPDOWN(w)   ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_DROPDOWN)
#define ZUI_IS_SCROLLVIEW(w) ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_SCROLLVIEW)
#define ZUI_IS_SCROLLER(w)   ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_SCROLLER)
#define ZUI_IS_SPLITVIEW(w)  ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_SPLITVIEW)
#define ZUI_IS_PROGRESSBAR(w) ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_PROGRESSBAR)
#define ZUI_IS_GRIDVIEW(w)   ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_GRIDVIEW)
#define ZUI_IS_MENUBAR(w)    ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_MENUBAR)
#define ZUI_IS_MENU(w)       ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_MENU)
#define ZUI_IS_MENUITEM(w)   ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_MENUITEM)
#define ZUI_IS_PIECHART(w)   ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_PIE_CHART)
#define ZUI_IS_BARCHART(w)   ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_BAR_CHART)
#define ZUI_IS_LINECHART(w)  ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_LINE_CHART)
#define ZUI_IS_CIRCULAR_PROGRESS(w) ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_CIRCULAR_PROGRESS)
#define ZUI_IS_RADIOBUTTON(w) ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_RADIOBUTTON)
#define ZUI_IS_ICON(w)       ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_ICON)
#define ZUI_IS_IMAGE(w)      ((w) && zui_widget_get_type(ZUI_WIDGET(w)) == ZUI_WIDGET_IMAGE)

#define ZUI_BUTTON(w)     (ZUI_IS_BUTTON(w) ? (ZuiButton *)(w) : NULL)
#define ZUI_TEXT(w)      (ZUI_IS_TEXT(w) ? (ZuiText *)(w) : NULL)
#define ZUI_PANEL(w)      (ZUI_IS_PANEL(w) ? (ZuiPanel *)(w) : NULL)
#define ZUI_CHECKBOX(w)   (ZUI_IS_CHECKBOX(w) ? (ZuiCheckbox *)(w) : NULL)
#define ZUI_TEXTINPUT(w)  (ZUI_IS_TEXTINPUT(w) ? (ZuiTextInput *)(w) : NULL)
#define ZUI_SLIDER(w)     (ZUI_IS_SLIDER(w) ? (ZuiSlider *)(w) : NULL)
#define ZUI_DROPDOWN(w)   (ZUI_IS_DROPDOWN(w) ? (ZuiDropdown *)(w) : NULL)
#define ZUI_SCROLLVIEW(w) (ZUI_IS_SCROLLVIEW(w) ? (ZuiScrollView *)(w) : NULL)
#define ZUI_SCROLLER(w)   (ZUI_IS_SCROLLER(w) ? (ZuiScroller *)(w) : NULL)
#define ZUI_SPLITVIEW(w)  (ZUI_IS_SPLITVIEW(w) ? (ZuiSplitView *)(w) : NULL)
#define ZUI_PROGRESSBAR(w) (ZUI_IS_PROGRESSBAR(w) ? (ZuiProgressBar *)(w) : NULL)
#define ZUI_GRIDVIEW(w)   (ZUI_IS_GRIDVIEW(w) ? (ZuiGridView *)(w) : NULL)
#define ZUI_MENUBAR(w)    (ZUI_IS_MENUBAR(w) ? (ZuiMenuBar *)(w) : NULL)
#define ZUI_MENU(w)       (ZUI_IS_MENU(w) ? (ZuiMenu *)(w) : NULL)
#define ZUI_MENUITEM(w)   (ZUI_IS_MENUITEM(w) ? (ZuiMenuItem *)(w) : NULL)
#define ZUI_PIECHART(w)   (ZUI_IS_PIECHART(w) ? (ZuiPieChart *)(w) : NULL)
#define ZUI_BARCHART(w)   (ZUI_IS_BARCHART(w) ? (ZuiBarChart *)(w) : NULL)
#define ZUI_LINECHART(w)  (ZUI_IS_LINECHART(w) ? (ZuiLineChart *)(w) : NULL)
#define ZUI_CIRCULAR_PROGRESS(w) (ZUI_IS_CIRCULAR_PROGRESS(w) ? (ZuiCircularProgress *)(w) : NULL)
#define ZUI_RADIOBUTTON(w) (ZUI_IS_RADIOBUTTON(w) ? (ZuiRadioButton *)(w) : NULL)
#define ZUI_ICON(w)       (ZUI_IS_ICON(w) ? (ZuiIcon *)(w) : NULL)
#define ZUI_IMAGE(w)      (ZUI_IS_IMAGE(w) ? (ZuiImage *)(w) : NULL)

void zui_widget_destroy(ZuiWidget *widget);
void zui_widget_add_child(ZuiWidget *parent, ZuiWidget *child);
void zui_widget_remove_child(ZuiWidget *parent, ZuiWidget *child);
void zui_widget_focus(ZuiWidget *widget);
void zui_widget_unfocus(ZuiWidget *widget);

ZuiWidget *zui_widget_get_parent(ZuiWidget *widget);
int zui_widget_get_child_count(ZuiWidget *widget);
ZuiWidget *zui_widget_get_child(ZuiWidget *widget, int index);
bool zui_widget_is_visible(ZuiWidget *widget);

void zui_widget_set_visible(ZuiWidget *widget, bool visible);
void zui_widget_set_background(ZuiWidget *widget, ZuiColor color);
void zui_widget_set_corner_radius(ZuiWidget *widget, float radius);
void zui_widget_set_cursor(ZuiWidget *widget, ZuiCursor cursor);
void zui_widget_set_fill(ZuiWidget *widget, bool fill_width, bool fill_height);
void zui_widget_set_size(ZuiWidget *widget, float width, float height);
void zui_widget_set_position(ZuiWidget *widget, float x, float y);
void zui_widget_set_padding(ZuiWidget *widget, float padding);
void zui_widget_set_spacing(ZuiWidget *widget, float spacing);
void zui_widget_set_layout(ZuiWidget *widget, ZuiLayoutDir direction);
void zui_widget_set_alignment(ZuiWidget *widget, ZuiAlign main_axis, ZuiAlign cross_axis);

#define zui_set_visible zui_widget_set_visible
#define zui_set_background zui_widget_set_background
#define zui_set_corner_radius zui_widget_set_corner_radius
#define zui_set_cursor zui_widget_set_cursor
#define zui_set_fill zui_widget_set_fill
#define zui_set_size zui_widget_set_size
#define zui_set_padding zui_widget_set_padding
#define zui_set_spacing zui_widget_set_spacing

ZuiButton *zui_button_create(const char *text);
void zui_button_set_size(ZuiButton *button, float width, float height);
void zui_button_set_color(ZuiButton *button, ZuiColor color);
void zui_button_set_alignment(ZuiButton* button, ZuiAlign align);
void zui_button_on_click(ZuiButton *button, ZuiClickCallback callback,
                          void *user_data);
ZuiWidget *zui_button_as_widget(ZuiButton *button);

ZuiText *zui_text_create(const char *text);
void zui_text_set_size(ZuiText *text, float size);
void zui_text_set_content(ZuiText *text, const char *content);
const char *zui_text_get_content(ZuiText *text);
void zui_text_set_font(ZuiText *text, ZuiFont *font);
void zui_text_set_color(ZuiText *text, ZuiColor color);
float zui_text_get_font_size(ZuiText *text);
ZuiWidget *zui_text_as_widget(ZuiText *text);

#define zui_get_font_size zui_text_get_font_size

ZuiCheckbox *zui_checkbox_create(const char *label);
void zui_checkbox_set_checked(ZuiCheckbox *checkbox, bool checked);
bool zui_checkbox_is_checked(ZuiCheckbox *checkbox);
void zui_checkbox_set_label(ZuiCheckbox *checkbox, const char *label);
void zui_checkbox_set_size(ZuiCheckbox *checkbox, float size);
void zui_checkbox_set_box_color(ZuiCheckbox *checkbox, ZuiColor unchecked,
                                 ZuiColor checked);
void zui_checkbox_set_icon_color(ZuiCheckbox *checkbox, ZuiColor color);
void zui_checkbox_set_icon(ZuiCheckbox *checkbox, const char *svg_path);
void zui_checkbox_on_change(ZuiCheckbox *checkbox, ZuiCheckboxCallback callback,
                             void *user_data);
ZuiWidget *zui_checkbox_as_widget(ZuiCheckbox *checkbox);

ZuiRadioGroup *zui_radiogroup_create(void);
void zui_radiogroup_destroy(ZuiRadioGroup *group);
void zui_radiogroup_on_change(ZuiRadioGroup *group, ZuiRadioCallback callback,
                               void *user_data);
ZuiRadioButton *zui_radiogroup_get_selected(ZuiRadioGroup *group);
int zui_radiogroup_get_selected_index(ZuiRadioGroup *group);
void zui_radiogroup_select(ZuiRadioGroup *group, size_t index);

ZuiRadioButton *zui_radiobutton_create(ZuiRadioGroup *group, const char *label);
void zui_radiobutton_set_colors(ZuiRadioButton *rb, ZuiColor circle,
                                 ZuiColor selected, ZuiColor dot);
void zui_radiobutton_set_label_color(ZuiRadioButton *rb, ZuiColor color);
void zui_radiobutton_set_size(ZuiRadioButton *rb, float size);
const char *zui_radiobutton_get_label(ZuiRadioButton *rb);
ZuiWidget *zui_radiobutton_as_widget(ZuiRadioButton *rb);

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

typedef void (*ZuiScrollViewCallback)(ZuiScrollView *sv, float x, float y,
                                       void *user_data);

ZuiScrollView *zui_scrollview_create(void);
void zui_scrollview_set_content(ZuiScrollView *sv, ZuiWidget *content);
void zui_scrollview_set_size(ZuiScrollView *sv, float width, float height);
void zui_scrollview_scroll_to(ZuiScrollView *sv, float x, float y);
void zui_scrollview_get_scroll(ZuiScrollView *sv, float *x, float *y);
void zui_scrollview_on_scroll(ZuiScrollView *sv, ZuiScrollViewCallback callback,
                               void *user_data);
ZuiWidget *zui_scrollview_as_widget(ZuiScrollView *sv);

ZuiScroller *zui_scroller_create(bool vertical);
void zui_scroller_set_size(ZuiScroller *scroller, float width, float height);
void zui_scroller_set_range(ZuiScroller *scroller, float content_size,
                             float viewport_size);
void zui_scroller_set_value(ZuiScroller *scroller, float value);
float zui_scroller_get_value(ZuiScroller *scroller);
void zui_scroller_on_change(ZuiScroller *scroller, ZuiScrollerCallback callback,
                             void *user_data);
ZuiWidget *zui_scroller_as_widget(ZuiScroller *scroller);

typedef enum ZuiSplitAnchor {
  ZUI_SPLIT_ANCHOR_AUTO,
  ZUI_SPLIT_ANCHOR_START,
  ZUI_SPLIT_ANCHOR_CENTER,
  ZUI_SPLIT_ANCHOR_END,
} ZuiSplitAnchor;

ZuiSplitView *zui_splitview_create(bool vertical);
void zui_splitview_set_vertical(ZuiSplitView *sv, bool vertical);
void zui_splitview_add_child(ZuiSplitView *sv, ZuiWidget *widget);
size_t zui_splitview_get_child_count(ZuiSplitView *sv);
void zui_splitview_set_handle_position(ZuiSplitView *sv, size_t index, float position);
float zui_splitview_get_handle_position(ZuiSplitView *sv, size_t index);
void zui_splitview_set_min_child_size(ZuiSplitView *sv, float min_size);
void zui_splitview_set_anchor(ZuiSplitView *sv, ZuiSplitAnchor anchor);
void zui_splitview_set_handle_size(ZuiSplitView *sv, float size);
void zui_splitview_set_grip_length(ZuiSplitView *sv, float length);
void zui_splitview_set_handle_color(ZuiSplitView *sv, ZuiColor color);
void zui_splitview_set_grip_color(ZuiSplitView *sv, ZuiColor normal,
                                   ZuiColor hover, ZuiColor drag);
ZuiWidget *zui_splitview_as_widget(ZuiSplitView *sv);

ZuiTextInput *zui_textinput_create(const char *placeholder);
void zui_textinput_set_text(ZuiTextInput *input, const char *text);
const char *zui_textinput_get_text(ZuiTextInput *input);
void zui_textinput_set_placeholder(ZuiTextInput *input, const char *placeholder);
void zui_textinput_set_size(ZuiTextInput *input, float width, float height);
void zui_textinput_set_colors(ZuiTextInput *input, ZuiColor background,
                               ZuiColor text, ZuiColor placeholder,
                               ZuiColor cursor, ZuiColor border);
void zui_textinput_on_change(ZuiTextInput *input, ZuiTextInputCallback callback,
                              void *user_data);
ZuiWidget *zui_textinput_as_widget(ZuiTextInput *input);

ZuiSlider *zui_slider_create(float min, float max, float value);
void zui_slider_set_value(ZuiSlider *slider, float value);
float zui_slider_get_value(ZuiSlider *slider);
void zui_slider_set_range(ZuiSlider *slider, float min, float max);
void zui_slider_set_size(ZuiSlider *slider, float width, float height);
void zui_slider_set_colors(ZuiSlider *slider, ZuiColor track, ZuiColor fill,
                            ZuiColor thumb);
void zui_slider_on_change(ZuiSlider *slider, ZuiSliderCallback callback,
                           void *user_data);
ZuiWidget *zui_slider_as_widget(ZuiSlider *slider);

ZuiDropdown *zui_dropdown_create(const char *placeholder);
void zui_dropdown_add_item(ZuiDropdown *dropdown, const char *item);
void zui_dropdown_clear_items(ZuiDropdown *dropdown);
void zui_dropdown_set_selected(ZuiDropdown *dropdown, int index);
int zui_dropdown_get_selected(ZuiDropdown *dropdown);
const char *zui_dropdown_get_selected_item(ZuiDropdown *dropdown);
void zui_dropdown_set_size(ZuiDropdown *dropdown, float width, float height);
void zui_dropdown_set_colors(ZuiDropdown *dropdown, ZuiColor background,
                              ZuiColor text, ZuiColor border);
void zui_dropdown_on_change(ZuiDropdown *dropdown, ZuiDropdownCallback callback,
                             void *user_data);
ZuiWidget *zui_dropdown_as_widget(ZuiDropdown *dropdown);

ZuiProgressBar *zui_progressbar_create(void);
void zui_progressbar_set_value(ZuiProgressBar *bar, float value);
float zui_progressbar_get_value(ZuiProgressBar *bar);
void zui_progressbar_set_size(ZuiProgressBar *bar, float width, float height);
void zui_progressbar_set_colors(ZuiProgressBar *bar, ZuiColor track,
                                 ZuiColor fill);
void zui_progressbar_set_corner_radius(ZuiProgressBar *bar, float radius);
ZuiWidget *zui_progressbar_as_widget(ZuiProgressBar *bar);

ZuiGridView *zui_gridview_create(int columns);
void zui_gridview_set_columns(ZuiGridView *gv, int columns);
void zui_gridview_set_size(ZuiGridView *gv, float width, float height);
void zui_gridview_set_cell_size(ZuiGridView *gv, float width, float height);
void zui_gridview_set_gap(ZuiGridView *gv, float gap_x, float gap_y);
void zui_gridview_set_padding(ZuiGridView *gv, float padding);
void zui_gridview_set_background(ZuiGridView *gv, ZuiColor color);
void zui_gridview_add_child(ZuiGridView *gv, ZuiWidget *child);
void zui_gridview_remove_child(ZuiGridView *gv, ZuiWidget *child);
void zui_gridview_clear(ZuiGridView *gv);
void zui_gridview_scroll_to(ZuiGridView *gv, float x, float y);
void zui_gridview_get_scroll(ZuiGridView *gv, float *x, float *y);
ZuiWidget *zui_gridview_as_widget(ZuiGridView *gv);

ZuiMenuBar *zui_menubar_create(void);
void zui_menubar_set_size(ZuiMenuBar *bar, float width, float height);
void zui_menubar_set_colors(ZuiMenuBar *bar, ZuiColor background, ZuiColor text);
void zui_menubar_add_menu(ZuiMenuBar *bar, ZuiMenu *menu);
ZuiWidget *zui_menubar_as_widget(ZuiMenuBar *bar);

ZuiMenu *zui_menu_create(const char *title);
void zui_menu_set_title(ZuiMenu *menu, const char *title);
void zui_menu_add_item(ZuiMenu *menu, ZuiMenuItem *item);
void zui_menu_add_separator(ZuiMenu *menu);
ZuiWidget *zui_menu_as_widget(ZuiMenu *menu);

ZuiMenuItem *zui_menuitem_create(const char *label);
void zui_menuitem_set_label(ZuiMenuItem *item, const char *label);
void zui_menuitem_set_shortcut(ZuiMenuItem *item, const char *shortcut);
void zui_menuitem_set_icon(ZuiMenuItem *item, const char *icon_path, float size);
void zui_menuitem_set_enabled(ZuiMenuItem *item, bool enabled);
void zui_menuitem_on_click(ZuiMenuItem *item, ZuiMenuItemCallback callback,
                            void *user_data);
ZuiWidget *zui_menuitem_as_widget(ZuiMenuItem *item);

typedef void (*ZuiPieChartCallback)(ZuiPieChart *chart, int slice_index,
                                     void *user_data);

ZuiPieChart *zui_piechart_create(void);
void zui_piechart_set_size(ZuiPieChart *chart, float size);
void zui_piechart_add_slice(ZuiPieChart *chart, float value, ZuiColor color);
void zui_piechart_add_slice_labeled(ZuiPieChart *chart, float value,
                                     ZuiColor color, const char *label);
void zui_piechart_clear(ZuiPieChart *chart);
void zui_piechart_set_hole_radius(ZuiPieChart *chart, float radius);
void zui_piechart_set_show_labels(ZuiPieChart *chart, bool show);
void zui_piechart_set_show_values(ZuiPieChart *chart, bool show);
void zui_piechart_on_hover(ZuiPieChart *chart, ZuiPieChartCallback callback,
                           void *user_data);
void zui_piechart_on_click(ZuiPieChart *chart, ZuiPieChartCallback callback,
                           void *user_data);
int zui_piechart_get_hovered_slice(ZuiPieChart *chart);
ZuiWidget *zui_piechart_as_widget(ZuiPieChart *chart);

typedef void (*ZuiBarChartCallback)(ZuiBarChart *chart, int bar_index,
                                     void *user_data);

ZuiBarChart *zui_barchart_create(void);
void zui_barchart_set_size(ZuiBarChart *chart, float width, float height);
void zui_barchart_add_bar(ZuiBarChart *chart, float value, ZuiColor color);
void zui_barchart_add_bar_labeled(ZuiBarChart *chart, float value,
                                   ZuiColor color, const char *label);
void zui_barchart_clear(ZuiBarChart *chart);
void zui_barchart_set_max_value(ZuiBarChart *chart, float max);
void zui_barchart_set_bar_spacing(ZuiBarChart *chart, float spacing);
void zui_barchart_set_corner_radius(ZuiBarChart *chart, float radius);
void zui_barchart_set_show_labels(ZuiBarChart *chart, bool show);
void zui_barchart_set_show_values(ZuiBarChart *chart, bool show);
void zui_barchart_on_hover(ZuiBarChart *chart, ZuiBarChartCallback callback,
                           void *user_data);
void zui_barchart_on_click(ZuiBarChart *chart, ZuiBarChartCallback callback,
                           void *user_data);
int zui_barchart_get_hovered_bar(ZuiBarChart *chart);
ZuiWidget *zui_barchart_as_widget(ZuiBarChart *chart);

ZuiLineChart *zui_linechart_create(void);
void zui_linechart_set_size(ZuiLineChart *chart, float width, float height);
void zui_linechart_add_point(ZuiLineChart *chart, float value);
void zui_linechart_clear(ZuiLineChart *chart);
void zui_linechart_set_max_value(ZuiLineChart *chart, float max);
void zui_linechart_set_line_color(ZuiLineChart *chart, ZuiColor color);
void zui_linechart_set_line_thickness(ZuiLineChart *chart, float thickness);
void zui_linechart_set_show_points(ZuiLineChart *chart, bool show);
ZuiWidget *zui_linechart_as_widget(ZuiLineChart *chart);

ZuiCircularProgress *zui_circularprogress_create(void);
void zui_circularprogress_set_size(ZuiCircularProgress *cp, float size);
void zui_circularprogress_set_value(ZuiCircularProgress *cp, float value);
float zui_circularprogress_get_value(ZuiCircularProgress *cp);
void zui_circularprogress_set_thickness(ZuiCircularProgress *cp, float thickness);
void zui_circularprogress_set_colors(ZuiCircularProgress *cp, ZuiColor track,
                                      ZuiColor fill);
void zui_circularprogress_set_text(ZuiCircularProgress *cp, const char *text);
void zui_circularprogress_set_show_percentage(ZuiCircularProgress *cp, bool show);
void zui_circularprogress_set_text_color(ZuiCircularProgress *cp, ZuiColor color);
void zui_circularprogress_set_text_size(ZuiCircularProgress *cp, float size);
void zui_circularprogress_on_click(ZuiCircularProgress *cp,
                                    ZuiMouseButtonCallback callback,
                                    void *user_data);
ZuiWidget *zui_circularprogress_as_widget(ZuiCircularProgress *cp);

typedef struct ZuiWindow ZuiWindow;
ZuiWidget *zui_window_close_button(ZuiWindow *window);
ZuiWidget *zui_window_minimize_button(ZuiWindow *window);
ZuiWidget *zui_window_maximize_button(ZuiWindow *window);
ZuiWidget *zui_window_hide_button(ZuiWindow *window);

void zui_label_set_font_size(ZuiText *label, float size);

#endif
