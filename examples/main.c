#include <zui/zui.h>
#include <stdio.h>

static void on_content_button_click(ZuiWidget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
    printf("Content button clicked!\n");
}

int main(void)
{
    if (!zui_init()) {
        fprintf(stderr, "Failed to initialize ZUI\n");
        return 1;
    }

    ZuiWindow *window = zui_window_create(900, 600, "ZUI Demo");
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        zui_shutdown();
        return 1;
    }

    zui_window_set_corner_radius(window, 12.0f);

    ZuiTitlebar *titlebar = zui_window_titlebar(window);

    ZuiLabel *title = zui_label_create("ZUI Demo");
    zui_titlebar_set_center(titlebar, (ZuiWidget *)title);

    zui_titlebar_set_end(titlebar, zui_window_hide_button(window));
    zui_titlebar_set_end(titlebar, zui_window_maximize_button(window));
    zui_titlebar_set_end(titlebar, zui_window_close_button(window));

    ZuiWidget *content = zui_window_content(window);

    ZuiLabel *hello = zui_label_create("Hello, ZUI!");
    zui_label_set_color(hello, ZUI_COLOR_HEX(0x4fc3f7));
    zui_widget_add_child(content, (ZuiWidget *)hello);

    ZuiButton *click_me = zui_button_create("Click Me");
    zui_button_set_size(click_me, 120.0f, 40.0f);
    zui_button_set_color(click_me, ZUI_COLOR_HEX(0x107c10));
    zui_button_on_click(click_me, on_content_button_click, NULL);
    zui_widget_add_child(content, (ZuiWidget *)click_me);

    zui_window_show(window);

    while (zui_window_running(window)) {
        zui_poll_events();
        zui_window_render(window);
    }

    zui_window_destroy(window);
    zui_shutdown();

    return 0;
}
