#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION

#include <zui/internal/icon_internal.h>
#include <nanosvg/nanosvg.h>
#include <nanosvg/nanosvgrast.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void icon_draw(ZuiWidget *widget, ZuiRenderer *renderer);
static void icon_destroy(ZuiWidget *widget);

static const ZuiWidgetVTable icon_vtable = {
    .draw = icon_draw,
    .destroy = icon_destroy,
};

ZuiIconSource *zui_icon_load_svg(const char *path)
{
    NSVGimage *svg = nsvgParseFromFile(path, "px", 96.0f);
    if (!svg) return NULL;

    ZuiIconSource *source = calloc(1, sizeof(ZuiIconSource));
    if (!source) {
        nsvgDelete(svg);
        return NULL;
    }

    float svg_size = svg->width > svg->height ? svg->width : svg->height;
    source->native_size = svg_size;

    int raster_size = (int)ceilf(svg_size);
    if (raster_size < 16) raster_size = 16;
    if (raster_size > 512) raster_size = 512;

    source->raster_width = raster_size;
    source->raster_height = raster_size;
    source->raster_data = malloc((size_t)(raster_size * raster_size * 4));

    if (!source->raster_data) {
        free(source);
        nsvgDelete(svg);
        return NULL;
    }

    NSVGrasterizer *rast = nsvgCreateRasterizer();
    if (!rast) {
        free(source->raster_data);
        free(source);
        nsvgDelete(svg);
        return NULL;
    }

    float scale = (float)raster_size / svg_size;
    nsvgRasterize(rast, svg, 0, 0, scale, source->raster_data,
                  raster_size, raster_size, raster_size * 4);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(svg);

    return source;
}

ZuiIconSource *zui_icon_load_svg_data(const char *svg_data)
{
    char *data_copy = strdup(svg_data);
    if (!data_copy) return NULL;

    NSVGimage *svg = nsvgParse(data_copy, "px", 96.0f);
    free(data_copy);

    if (!svg) return NULL;

    ZuiIconSource *source = calloc(1, sizeof(ZuiIconSource));
    if (!source) {
        nsvgDelete(svg);
        return NULL;
    }

    float svg_size = svg->width > svg->height ? svg->width : svg->height;
    source->native_size = svg_size;

    int raster_size = (int)ceilf(svg_size);
    if (raster_size < 16) raster_size = 16;
    if (raster_size > 512) raster_size = 512;

    source->raster_width = raster_size;
    source->raster_height = raster_size;
    source->raster_data = malloc((size_t)(raster_size * raster_size * 4));

    if (!source->raster_data) {
        free(source);
        nsvgDelete(svg);
        return NULL;
    }

    NSVGrasterizer *rast = nsvgCreateRasterizer();
    if (!rast) {
        free(source->raster_data);
        free(source);
        nsvgDelete(svg);
        return NULL;
    }

    float scale = (float)raster_size / svg_size;
    nsvgRasterize(rast, svg, 0, 0, scale, source->raster_data,
                  raster_size, raster_size, raster_size * 4);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(svg);

    return source;
}

void zui_icon_source_destroy(ZuiIconSource *source)
{
    if (!source) return;
    free(source->raster_data);
    free(source);
}

ZuiIcon *zui_icon_create(ZuiIconSource *source, float size)
{
    if (!source) return NULL;

    ZuiIcon *icon = (ZuiIcon *)zui_widget_create(sizeof(ZuiIcon),
                                                  ZUI_WIDGET_ICON,
                                                  &icon_vtable);
    if (!icon) return NULL;

    icon->source = source;
    icon->size = size;
    icon->tint = ZUI_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    icon->texture_dirty = true;

    icon->base.preferred_size.width = size;
    icon->base.preferred_size.height = size;
    icon->base.min_size.width = size;
    icon->base.min_size.height = size;

    return icon;
}

void zui_icon_set_size(ZuiIcon *icon, float size)
{
    if (!icon) return;
    icon->size = size;
    icon->base.preferred_size.width = size;
    icon->base.preferred_size.height = size;
    icon->base.min_size.width = size;
    icon->base.min_size.height = size;
}

void zui_icon_set_color(ZuiIcon *icon, ZuiColor color)
{
    if (!icon) return;
    icon->tint = color;
}

void zui_icon_rasterize(ZuiIcon *icon)
{
    if (!icon || !icon->source) return;

    if (icon->texture.id) {
        zui_texture_destroy(&icon->texture);
    }

    icon->texture = zui_texture_create(icon->source->raster_data,
                                        icon->source->raster_width,
                                        icon->source->raster_height);
    icon->texture_dirty = false;
}

static void icon_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
    ZuiIcon *icon = (ZuiIcon *)widget;
    if (!icon->source) return;

    if (icon->texture_dirty || !icon->texture.id) {
        zui_icon_rasterize(icon);
    }

    if (!icon->texture.id) return;

    ZuiRect rect = ZUI_RECT(widget->bounds.x, widget->bounds.y,
                             icon->size, icon->size);
    zui_renderer_draw_texture(renderer, &icon->texture, rect, icon->tint);
}

static void icon_destroy(ZuiWidget *widget)
{
    ZuiIcon *icon = (ZuiIcon *)widget;
    if (icon->texture.id) {
        zui_texture_destroy(&icon->texture);
    }
}
