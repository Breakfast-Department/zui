#ifndef ZUI_ICON_INTERNAL_H
#define ZUI_ICON_INTERNAL_H

#include "widget_internal.h"
#include "renderer.h"
#include <zui/icon.h>

typedef struct ZuiIconSource {
    unsigned char *raster_data;
    int raster_width;
    int raster_height;
    float native_size;
} ZuiIconSource;

struct ZuiIcon {
    ZuiWidget base;
    ZuiIconSource *source;
    ZuiTexture texture;
    ZuiColor tint;
    float size;
    bool texture_dirty;
};

ZuiIconSource *zui_icon_source_create(void);
void zui_icon_rasterize(ZuiIcon *icon);

#endif
