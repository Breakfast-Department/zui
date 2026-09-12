#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <webp/decode.h>

#include <zui/internal/widget_internal.h>
#include <zui/internal/renderer.h>
#include <zui/image.h>
#include <zui/resource.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

  if (!image->texture.id) {
    return;
  }

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

static bool is_webp_file(const char *path)
{
  if (!path) return false;
  size_t len = strlen(path);
  if (len < 5) return false;
  const char *ext = path + len - 5;
  return (strcmp(ext, ".webp") == 0 || strcmp(ext, ".WEBP") == 0);
}

static bool is_webp_data(const unsigned char *data, int len)
{
  if (!data || len < 12) return false;
  return (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F' &&
          data[8] == 'W' && data[9] == 'E' && data[10] == 'B' && data[11] == 'P');
}

static unsigned char *load_webp_file(const char *path, int *width, int *height)
{
  FILE *f = fopen(path, "rb");
  if (!f) return NULL;

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (size <= 0) {
    fclose(f);
    return NULL;
  }

  uint8_t *file_data = malloc((size_t)size);
  if (!file_data) {
    fclose(f);
    return NULL;
  }

  if (fread(file_data, 1, (size_t)size, f) != (size_t)size) {
    free(file_data);
    fclose(f);
    return NULL;
  }
  fclose(f);

  unsigned char *pixels = WebPDecodeRGBA(file_data, (size_t)size, width, height);
  free(file_data);

  return pixels;
}

static unsigned char *load_webp_memory(const unsigned char *data, int len, int *width, int *height)
{
  if (!data || len <= 0) return NULL;
  return WebPDecodeRGBA(data, (size_t)len, width, height);
}

ZuiImage *zui_image_create(const char *path)
{
  if (!path) {
    return NULL;
  }

  if (zui_is_resource_path(path)) {
    size_t size;
    const unsigned char *res_data = zui_resource_get(path, &size);
    if (res_data && size > 0) {
      return zui_image_create_from_memory(res_data, (int)size);
    }
    return NULL;
  }

  int width, height;
  unsigned char *data = NULL;
  bool is_webp = is_webp_file(path);

  if (is_webp) {
    data = load_webp_file(path, &width, &height);
  } else {
    int channels;
    stbi_set_flip_vertically_on_load(0);
    data = stbi_load(path, &width, &height, &channels, 4);
  }

  if (!data) {
    return NULL;
  }

  ZuiImage *image = (ZuiImage *)zui_widget_create(
    sizeof(ZuiImage), ZUI_WIDGET_IMAGE, &image_vtable);

  if (!image) {
    if (is_webp) {
      WebPFree(data);
    } else {
      stbi_image_free(data);
    }
    return NULL;
  }

  image->img_width = width;
  image->img_height = height;
  image->owns_data = true;

  image->texture = zui_texture_create(data, width, height);

  if (is_webp) {
    WebPFree(data);
  } else {
    stbi_image_free(data);
  }

  image->base.preferred_size.width = (float)width;
  image->base.preferred_size.height = (float)height;

  return image;
}

ZuiImage *zui_image_create_from_memory(const unsigned char *data, int len)
{
  if (!data || len <= 0) {
    return NULL;
  }

  int width, height;
  unsigned char *pixels = NULL;
  bool is_webp = is_webp_data(data, len);

  if (is_webp) {
    pixels = load_webp_memory(data, len, &width, &height);
  } else {
    int channels;
    stbi_set_flip_vertically_on_load(0);
    pixels = stbi_load_from_memory(data, len, &width, &height, &channels, 4);
  }

  if (!pixels) {
    return NULL;
  }

  ZuiImage *image = (ZuiImage *)zui_widget_create(
    sizeof(ZuiImage), ZUI_WIDGET_IMAGE, &image_vtable);

  if (!image) {
    if (is_webp) {
      WebPFree(pixels);
    } else {
      stbi_image_free(pixels);
    }
    return NULL;
  }

  image->img_width = width;
  image->img_height = height;
  image->owns_data = true;

  image->texture = zui_texture_create(pixels, width, height);

  if (is_webp) {
    WebPFree(pixels);
  } else {
    stbi_image_free(pixels);
  }

  image->base.preferred_size.width = (float)width;
  image->base.preferred_size.height = (float)height;

  return image;
}

void zui_image_destroy(ZuiImage *image)
{
  if (!image) {
    return;
  }

  zui_widget_destroy((ZuiWidget *)image);
}

void zui_image_set_size(ZuiImage *image, float width, float height)
{
  if (!image) {
    return;
  }

  image->base.preferred_size.width = width;
  image->base.preferred_size.height = height;
}

void zui_image_set_visible(ZuiImage *image, bool visible)
{
  if (!image) {
    return;
  }

  image->base.visible = visible;
}

ZuiWidget *zui_image_as_widget(ZuiImage *image)
{
  return image ? (ZuiWidget *)image : NULL;
}

ZuiWidget *zui_image_new(const char *path)
{
  return (ZuiWidget *)zui_image_create(path);
}

ZuiTexture zui_image_get_texture(ZuiImage *image)
{
  if (!image) {
    return (ZuiTexture){0};
  }

  return image->texture;
}

void zui_image_detach_texture(ZuiImage *image)
{
  if (!image) {
    return;
  }

  image->texture = (ZuiTexture){0};
}
