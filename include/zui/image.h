#ifndef ZUI_IMAGE_H
#define ZUI_IMAGE_H

#include <stdbool.h>

typedef struct ZuiImage ZuiImage;
typedef struct ZuiWidget ZuiWidget;

ZuiImage *zui_image_create(const char *path);
ZuiImage *zui_image_create_from_memory(const unsigned char *data, int len);
void zui_image_destroy(ZuiImage *image);

void zui_image_set_size(ZuiImage *image, float width, float height);
void zui_image_set_visible(ZuiImage *image, bool visible);

ZuiWidget *zui_image_widget(ZuiImage *image);

#endif
