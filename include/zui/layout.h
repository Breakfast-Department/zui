#ifndef ZUI_LAYOUT_H
#define ZUI_LAYOUT_H

typedef struct ZuiWidget ZuiWidget;

typedef enum ZuiLayoutDir {
    ZUI_LAYOUT_HORIZONTAL,
    ZUI_LAYOUT_VERTICAL,
} ZuiLayoutDir;

typedef enum ZuiAlign {
    ZUI_ALIGN_START,
    ZUI_ALIGN_CENTER,
    ZUI_ALIGN_END,
} ZuiAlign;

void zui_widget_add_child(ZuiWidget *parent, ZuiWidget *child);
void zui_widget_set_padding(ZuiWidget *widget, float padding);
void zui_widget_set_spacing(ZuiWidget *widget, float spacing);

#endif
