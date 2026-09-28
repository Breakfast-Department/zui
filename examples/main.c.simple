#include "zui/color.h"
#include "zui/widget.h"
#include <stdio.h>
#include <zui/zui.h>

int main(void) {
  if (!zui_init()) {
    printf("Failed initialize zui!");
    return -1;
  }

  ZuiWindow* window = zui_window_create(800, 600, "Zui Examples");
  zui_window_set_background_color(window, ZUI_COLOR_HEX(0x121212));

  ZuiWindowDecoration* decoration = zui_default_window_decoration(window);
  zui_window_decoration_set_title(decoration, "Zui Examples");

  zui_window_show(window);

  while(zui_window_running(window)) {
    zui_window_render(window);
    zui_poll_events();
  }

  zui_window_destroy(window);
  zui_shutdown();

  return 0;
}
