#ifndef ZUI_ICON_H
#define ZUI_ICON_H

#include <stdbool.h>
#include <stddef.h>
#include "color.h"

typedef struct ZuiIcon ZuiIcon;
typedef struct ZuiIconSource ZuiIconSource;

ZuiIconSource *zui_icon_load_svg(const char *path);
ZuiIconSource *zui_icon_load_svg_data(const char *svg_data);
void zui_icon_source_destroy(ZuiIconSource *source);

ZuiIcon *zui_icon_create(ZuiIconSource *source, float size);
void zui_icon_set_size(ZuiIcon *icon, float size);
void zui_icon_set_color(ZuiIcon *icon, ZuiColor color);

#endif
