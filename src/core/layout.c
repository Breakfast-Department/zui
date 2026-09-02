#include <zui/internal/widget_internal.h>

void zui_widget_set_visible(ZuiWidget *widget, bool visible)
{
    if (widget) widget->visible = visible;
}

void zui_widget_set_cursor(ZuiWidget *widget, ZuiCursor cursor)
{
    if (widget) widget->cursor = cursor;
}
