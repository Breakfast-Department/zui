#include <zui/resource.h>
#include <string.h>

#include "zui_internal_resources.h"

bool zui_is_resource_path(const char *path)
{
    if (!path) return false;
    return (strncmp(path, "res:", 4) == 0);
}

__attribute__((weak))
const unsigned char *zui_app_resource_get(const char *path, size_t *size)
{
    (void)path;
    if (size) *size = 0;
    return NULL;
}

__attribute__((weak))
bool zui_app_resource_exists(const char *path)
{
    (void)path;
    return false;
}

const unsigned char *zui_resource_get(const char *path, size_t *size)
{
    const unsigned char *data = zui_app_resource_get(path, size);
    if (data) return data;

    return zui_internal_resource_get(path, size);
}

bool zui_resource_exists(const char *path)
{
    if (zui_app_resource_exists(path)) return true;

    return zui_internal_resource_exists(path);
}
