#include <zui/zui.h>
#include <stdio.h>

static void on_button_click(ZuiWidget *widget, void *user_data)
{
    (void)widget;
    const char *name = user_data;
    printf("%s clicked!\n", name);
}

int main(void)
{
    if (!zui_init()) {
        fprintf(stderr, "Failed to initialize ZUI\n");
        return 1;
    }

    ZuiWindow *window = zui_window_create(900, 600, "ZUI Panel Demo");
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        zui_shutdown();
        return 1;
    }

    zui_window_set_corner_radius(window, 12.0f);

    ZuiTitlebar *titlebar = zui_window_titlebar(window);

    ZuiLabel *title = zui_label_create("Panel Layout Demo");
    zui_titlebar_set_center(titlebar, (ZuiWidget *)title);

    zui_titlebar_set_end(titlebar, zui_window_hide_button(window));
    zui_titlebar_set_end(titlebar, zui_window_maximize_button(window));
    zui_titlebar_set_end(titlebar, zui_window_close_button(window));

    ZuiWidget *content = zui_window_content(window);

    ZuiPanel *main_panel = zui_panel_create();
    zui_panel_set_layout(main_panel, ZUI_LAYOUT_HORIZONTAL);
    zui_panel_set_fill(main_panel, true, true);
    zui_panel_set_spacing(main_panel, 10.0f);
    zui_panel_set_padding(main_panel, 10.0f, 10.0f, 10.0f, 10.0f);
    zui_widget_add_child(content, zui_panel_as_widget(main_panel));

    ZuiPanel *sidebar = zui_panel_create();
    zui_panel_set_layout(sidebar, ZUI_LAYOUT_VERTICAL);
    zui_panel_set_size(sidebar, 200.0f, 0.0f);
    zui_widget_set_fill(zui_panel_as_widget(sidebar), false, true);
    zui_panel_set_background(sidebar, ZUI_COLOR_HEX(0x2d2d2d));
    zui_panel_set_corner_radius(sidebar, 8.0f);
    zui_panel_set_padding(sidebar, 15.0f, 15.0f, 15.0f, 15.0f);
    zui_panel_set_spacing(sidebar, 8.0f);
    zui_panel_add_child(main_panel, zui_panel_as_widget(sidebar));

    ZuiLabel *sidebar_title = zui_label_create("Navigation");
    zui_label_set_size(sidebar_title, 18.0f);
    zui_label_set_color(sidebar_title, ZUI_COLOR_HEX(0xcccccc));
    zui_panel_add_child(sidebar, (ZuiWidget *)sidebar_title);

    const char *nav_items[] = {"Dashboard", "Settings", "Profile", "Help"};
    for (int i = 0; i < 4; i++) {
        ZuiButton *btn = zui_button_create(nav_items[i]);
        zui_button_set_size(btn, 170.0f, 36.0f);
        zui_button_set_color(btn, ZUI_COLOR_HEX(0x3d3d3d));
        zui_button_on_click(btn, on_button_click, (void *)nav_items[i]);
        zui_panel_add_child(sidebar, (ZuiWidget *)btn);
    }

    ZuiPanel *content_panel = zui_panel_create();
    zui_panel_set_layout(content_panel, ZUI_LAYOUT_VERTICAL);
    zui_widget_set_fill(zui_panel_as_widget(content_panel), true, true);
    zui_panel_set_background(content_panel, ZUI_COLOR_HEX(0x252525));
    zui_panel_set_corner_radius(content_panel, 8.0f);
    zui_panel_set_padding(content_panel, 20.0f, 20.0f, 20.0f, 20.0f);
    zui_panel_set_spacing(content_panel, 15.0f);
    zui_panel_add_child(main_panel, zui_panel_as_widget(content_panel));

    ZuiLabel *content_title = zui_label_create("Welcome to ZUI");
    zui_label_set_size(content_title, 28.0f);
    zui_label_set_color(content_title, ZUI_COLOR_HEX(0xffffff));
    zui_panel_add_child(content_panel, (ZuiWidget *)content_title);

    ZuiLabel *description = zui_label_create("A lightweight modern GUI toolkit");
    zui_label_set_size(description, 16.0f);
    zui_label_set_color(description, ZUI_COLOR_HEX(0x888888));
    zui_panel_add_child(content_panel, (ZuiWidget *)description);

    ZuiPanel *button_row = zui_panel_create();
    zui_panel_set_layout(button_row, ZUI_LAYOUT_HORIZONTAL);
    zui_panel_set_size(button_row, 400.0f, 50.0f);
    zui_panel_set_spacing(button_row, 10.0f);
    zui_panel_add_child(content_panel, zui_panel_as_widget(button_row));

    ZuiButton *primary_btn = zui_button_create("Primary");
    zui_button_set_size(primary_btn, 100.0f, 40.0f);
    zui_button_set_color(primary_btn, ZUI_COLOR_HEX(0x6E6ADE));
    zui_button_on_click(primary_btn, on_button_click, "Primary");
    zui_panel_add_child(button_row, (ZuiWidget *)primary_btn);

    ZuiButton *secondary_btn = zui_button_create("Secondary");
    zui_button_set_size(secondary_btn, 100.0f, 40.0f);
    zui_button_set_color(secondary_btn, ZUI_COLOR_HEX(0x4a4a4a));
    zui_button_on_click(secondary_btn, on_button_click, "Secondary");
    zui_panel_add_child(button_row, (ZuiWidget *)secondary_btn);

    ZuiButton *danger_btn = zui_button_create("Danger");
    zui_button_set_size(danger_btn, 100.0f, 40.0f);
    zui_button_set_color(danger_btn, ZUI_COLOR_HEX(0xe81123));
    zui_button_on_click(danger_btn, on_button_click, "Danger");
    zui_panel_add_child(button_row, (ZuiWidget *)danger_btn);

    zui_window_show(window);

    while (zui_window_running(window)) {
        zui_poll_events();
        zui_window_render(window);
    }

    zui_window_destroy(window);
    zui_shutdown();

    return 0;
}
