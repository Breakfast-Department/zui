#ifndef ZUI_TYPES_H
#define ZUI_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct ZuiWidget ZuiWidget;
typedef struct ZuiWindow ZuiWindow;
typedef struct ZuiWindowDecoration ZuiWindowDecoration;
typedef struct ZuiButton ZuiButton;
typedef struct ZuiText ZuiText;
typedef struct ZuiContainer ZuiContainer;
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
typedef struct ZuiLineChart ZuiLineChart;
typedef struct ZuiCircularProgress ZuiCircularProgress;
typedef struct ZuiVideo ZuiVideo;

#include <zui/color.h>
#include <zui/layout.h>

typedef struct ZuiPoint {
  float x, y;
} ZuiPoint;

typedef struct ZuiSize {
  float width, height;
} ZuiSize;

typedef struct ZuiBounds {
  float x, y, width, height;
} ZuiBounds;

typedef struct ZuiIcon ZuiIcon;

#include <zui/widget.h>

typedef void (*ZuiClickCallback)(ZuiWidget *widget, void *user_data);
typedef void (*ZuiMouseButtonCallback)(ZuiWidget *widget, uint32_t button,
                                        void *user_data);
typedef void (*ZuiDrawCallback)(ZuiWidget *widget, void *renderer);
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
typedef void (*ZuiMenuItemCallback)(ZuiMenuItem *item, void *user_data);

#endif
