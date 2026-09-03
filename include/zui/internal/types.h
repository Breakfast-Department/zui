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
typedef struct ZuiPanel ZuiPanel;

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

typedef enum ZuiWidgetType {
    ZUI_WIDGET_CONTAINER,
    ZUI_WIDGET_WINDOW,
    ZUI_WIDGET_TITLEBAR,
    ZUI_WIDGET_BUTTON,
    ZUI_WIDGET_LABEL,
    ZUI_WIDGET_ICON,
    ZUI_WIDGET_IMAGE,
    ZUI_WIDGET_PANEL,
} ZuiWidgetType;

typedef void (*ZuiClickCallback)(ZuiWidget *widget, void *user_data);
typedef void (*ZuiDrawCallback)(ZuiWidget *widget, void *renderer);

#endif
