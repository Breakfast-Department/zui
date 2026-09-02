#ifndef ZUI_APP_H
#define ZUI_APP_H

#include <stdbool.h>

bool zui_init(void);
void zui_shutdown(void);
void zui_poll_events(void);

#endif
