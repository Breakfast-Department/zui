#ifndef ZUI_TYPES_H
#define ZUI_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct ZuiWidget ZuiWidget;
typedef struct ZuiWindow ZuiWindow;
typedef struct ZuiTitlebar ZuiTitlebar;
typedef struct ZuiButton ZuiButton;
typedef struct ZuiLabel ZuiLabel;
typedef struct ZuiContainer ZuiContainer;

#include <zui/color.h>

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

typedef enum ZuiWidgetType {
    ZUI_WIDGET_CONTAINER,
    ZUI_WIDGET_WINDOW,
    ZUI_WIDGET_TITLEBAR,
    ZUI_WIDGET_BUTTON,
    ZUI_WIDGET_LABEL,
    ZUI_WIDGET_ICON,
    ZUI_WIDGET_IMAGE,
} ZuiWidgetType;

typedef enum ZuiLayoutDir {
    ZUI_LAYOUT_HORIZONTAL,
    ZUI_LAYOUT_VERTICAL,
} ZuiLayoutDir;

typedef enum ZuiAlign {
    ZUI_ALIGN_START,
    ZUI_ALIGN_CENTER,
    ZUI_ALIGN_END,
} ZuiAlign;

typedef void (*ZuiClickCallback)(ZuiWidget *widget, void *user_data);
typedef void (*ZuiDrawCallback)(ZuiWidget *widget, void *renderer);

#endif
