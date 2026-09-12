#ifndef ZUI_APP_H
#define ZUI_APP_H

#include <stdbool.h>
#include <stddef.h>

bool zui_init(void);
void zui_shutdown(void);
void zui_poll_events(void);

const char *zui_get_base_path(void);
bool zui_resolve_asset_path(const char *relative_path, char *out, size_t size);

const char *zui_version(void);
void zui_version_info(int *major, int *minor, int *patch);

#endif
