#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <zui/internal/widget_internal.h>
#include <zui/internal/renderer.h>
#include <zui/image.h>
#include <stdlib.h>
#include <string.h>

struct ZuiImage {
    ZuiWidget base;
    ZuiTexture texture;
    int img_width;
    int img_height;
    bool owns_data;
};

static void image_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
    ZuiImage *image = (ZuiImage *)widget;

    if (!image->texture.id) return;

    zui_renderer_draw_texture(renderer, &image->texture,
        ZUI_RECT(widget->bounds.x, widget->bounds.y,
                  widget->bounds.width, widget->bounds.height),
        ZUI_COLOR_RGB(1.0f, 1.0f, 1.0f));
}

static void image_destroy(ZuiWidget *widget)
{
    ZuiImage *image = (ZuiImage *)widget;
    if (image->texture.id) {
        zui_texture_destroy(&image->texture);
    }
}

static const ZuiWidgetVTable image_vtable = {
    .draw = image_draw,
    .destroy = image_destroy,
};

ZuiImage *zui_image_create(const char *path)
{
    if (!path) return NULL;

    int width, height, channels;
    stbi_set_flip_vertically_on_load(0);
    unsigned char *data = stbi_load(path, &width, &height, &channels, 4);
    if (!data) return NULL;

    ZuiImage *image = (ZuiImage *)zui_widget_create(
        sizeof(ZuiImage), ZUI_WIDGET_IMAGE, &image_vtable);
    if (!image) {
        stbi_image_free(data);
        return NULL;
    }

    image->img_width = width;
    image->img_height = height;
    image->owns_data = true;

    image->texture = zui_texture_create(data, width, height);
    stbi_image_free(data);

    image->base.preferred_size.width = (float)width;
    image->base.preferred_size.height = (float)height;

    return image;
}

ZuiImage *zui_image_create_from_memory(const unsigned char *data, int len)
{
    if (!data || len <= 0) return NULL;

    int width, height, channels;
    stbi_set_flip_vertically_on_load(0);
    unsigned char *pixels = stbi_load_from_memory(data, len, &width, &height, &channels, 4);
    if (!pixels) return NULL;

    ZuiImage *image = (ZuiImage *)zui_widget_create(
        sizeof(ZuiImage), ZUI_WIDGET_IMAGE, &image_vtable);
    if (!image) {
        stbi_image_free(pixels);
        return NULL;
    }

    image->img_width = width;
    image->img_height = height;
    image->owns_data = true;

    image->texture = zui_texture_create(pixels, width, height);
    stbi_image_free(pixels);

    image->base.preferred_size.width = (float)width;
    image->base.preferred_size.height = (float)height;

    return image;
}

void zui_image_destroy(ZuiImage *image)
{
    if (!image) return;
    zui_widget_destroy((ZuiWidget *)image);
}

void zui_image_set_size(ZuiImage *image, float width, float height)
{
    if (!image) return;
    image->base.preferred_size.width = width;
    image->base.preferred_size.height = height;
}

void zui_image_set_visible(ZuiImage *image, bool visible)
{
    if (!image) return;
    image->base.visible = visible;
}

ZuiWidget *zui_image_widget(ZuiImage *image)
{
    return image ? (ZuiWidget *)image : NULL;
}
