#include <zui/internal/widget_internal.h>

typedef struct ZuiEdgeInsets {
    float top;
    float right;
    float bottom;
    float left;
} ZuiEdgeInsets;

struct ZuiPanel {
    ZuiWidget base;
    ZuiLayoutDir layout_dir;
    ZuiEdgeInsets padding;
    float spacing;
    ZuiAlign main_align;
    ZuiAlign cross_align;
};

static void panel_layout(ZuiWidget *widget)
{
    ZuiPanel *panel = (ZuiPanel *)widget;

    float content_x = widget->bounds.x + panel->padding.left;
    float content_y = widget->bounds.y + panel->padding.top;
    float content_width = widget->bounds.width - panel->padding.left - panel->padding.right;
    float content_height = widget->bounds.height - panel->padding.top - panel->padding.bottom;

    if (widget->child_count == 0) return;

    for (int i = 0; i < widget->child_count; i++) {
        ZuiWidget *child = widget->children[i];
        if (child->fill_width) {
            child->preferred_size.width = content_width;
        }
        if (child->fill_height) {
            child->preferred_size.height = content_height;
        }
    }

    float total_fixed = 0.0f;
    int expand_count = 0;

    for (int i = 0; i < widget->child_count; i++) {
        ZuiWidget *child = widget->children[i];
        if (!child->visible) continue;

        if (panel->layout_dir == ZUI_LAYOUT_HORIZONTAL) {
            if (child->expand) {
                expand_count++;
            } else {
                total_fixed += child->preferred_size.width;
            }
        } else {
            if (child->expand) {
                expand_count++;
            } else {
                total_fixed += child->preferred_size.height;
            }
        }
    }

    int visible_count = 0;
    for (int i = 0; i < widget->child_count; i++) {
        if (widget->children[i]->visible) visible_count++;
    }

    float total_spacing = (visible_count > 1) ? panel->spacing * (float)(visible_count - 1) : 0;
    float available;
    if (panel->layout_dir == ZUI_LAYOUT_HORIZONTAL) {
        available = content_width - total_fixed - total_spacing;
    } else {
        available = content_height - total_fixed - total_spacing;
    }

    float expand_size = (expand_count > 0) ? available / (float)expand_count : 0;
    if (expand_size < 0) expand_size = 0;

    float total_content = total_fixed + total_spacing;
    if (expand_count > 0) {
        total_content += expand_size * (float)expand_count;
    }

    float offset = 0;
    if (panel->layout_dir == ZUI_LAYOUT_HORIZONTAL) {
        switch (panel->main_align) {
        case ZUI_ALIGN_CENTER:
            offset = (content_width - total_content) / 2;
            break;
        case ZUI_ALIGN_END:
            offset = content_width - total_content;
            break;
        default:
            offset = 0;
            break;
        }
    } else {
        switch (panel->main_align) {
        case ZUI_ALIGN_CENTER:
            offset = (content_height - total_content) / 2;
            break;
        case ZUI_ALIGN_END:
            offset = content_height - total_content;
            break;
        default:
            offset = 0;
            break;
        }
    }

    float pos = offset;

    for (int i = 0; i < widget->child_count; i++) {
        ZuiWidget *child = widget->children[i];
        if (!child->visible) continue;

        float child_width, child_height;
        float child_x, child_y;

        if (panel->layout_dir == ZUI_LAYOUT_HORIZONTAL) {
            child_width = child->expand ? expand_size : child->preferred_size.width;
            child_height = child->preferred_size.height;

            switch (panel->cross_align) {
            case ZUI_ALIGN_CENTER:
                child_y = content_y + (content_height - child_height) / 2;
                break;
            case ZUI_ALIGN_END:
                child_y = content_y + content_height - child_height;
                break;
            default:
                child_y = content_y;
                break;
            }

            child_x = content_x + pos;
            pos += child_width + panel->spacing;
        } else {
            child_width = child->preferred_size.width;
            child_height = child->expand ? expand_size : child->preferred_size.height;

            switch (panel->cross_align) {
            case ZUI_ALIGN_CENTER:
                child_x = content_x + (content_width - child_width) / 2;
                break;
            case ZUI_ALIGN_END:
                child_x = content_x + content_width - child_width;
                break;
            default:
                child_x = content_x;
                break;
            }

            child_y = content_y + pos;
            pos += child_height + panel->spacing;
        }

        zui_widget_set_bounds(child, child_x, child_y, child_width, child_height);
    }
}

static void panel_destroy(ZuiWidget *widget)
{
    (void)widget;
}

static bool panel_hit_test(ZuiWidget *widget, float x, float y)
{
    (void)widget;
    (void)x;
    (void)y;
    return true;
}

static const ZuiWidgetVTable panel_vtable = {
    .layout = panel_layout,
    .destroy = panel_destroy,
    .hit_test = panel_hit_test,
};

ZuiPanel *zui_panel_create(void)
{
    ZuiPanel *panel = (ZuiPanel *)zui_widget_create(
        sizeof(ZuiPanel), ZUI_WIDGET_PANEL, &panel_vtable);
    if (!panel) return NULL;

    panel->layout_dir = ZUI_LAYOUT_VERTICAL;
    panel->padding = (ZuiEdgeInsets){0, 0, 0, 0};
    panel->spacing = 0;
    panel->main_align = ZUI_ALIGN_START;
    panel->cross_align = ZUI_ALIGN_START;

    panel->base.background = ZUI_COLOR(0, 0, 0, 0);
    panel->base.needs_layout = true;

    return panel;
}

void zui_panel_destroy(ZuiPanel *panel)
{
    if (!panel) return;
    zui_widget_destroy(&panel->base);
}

void zui_panel_set_layout(ZuiPanel *panel, ZuiLayoutDir direction)
{
    if (!panel) return;
    panel->layout_dir = direction;
    zui_widget_invalidate(&panel->base);
}

void zui_panel_set_padding(ZuiPanel *panel, float top, float right, float bottom, float left)
{
    if (!panel) return;
    panel->padding.top = top;
    panel->padding.right = right;
    panel->padding.bottom = bottom;
    panel->padding.left = left;
    zui_widget_invalidate(&panel->base);
}

void zui_panel_set_spacing(ZuiPanel *panel, float spacing)
{
    if (!panel) return;
    panel->spacing = spacing;
    zui_widget_invalidate(&panel->base);
}

void zui_panel_set_alignment(ZuiPanel *panel, ZuiAlign main_axis, ZuiAlign cross_axis)
{
    if (!panel) return;
    panel->main_align = main_axis;
    panel->cross_align = cross_axis;
    zui_widget_invalidate(&panel->base);
}

void zui_panel_set_background(ZuiPanel *panel, ZuiColor color)
{
    if (!panel) return;
    panel->base.background = color;
}

void zui_panel_set_corner_radius(ZuiPanel *panel, float radius)
{
    if (!panel) return;
    panel->base.corner_radius = radius;
}

void zui_panel_set_size(ZuiPanel *panel, float width, float height)
{
    if (!panel) return;
    panel->base.preferred_size.width = width;
    panel->base.preferred_size.height = height;
    zui_widget_invalidate(&panel->base);
}

void zui_panel_set_fill(ZuiPanel *panel, bool fill_width, bool fill_height)
{
    if (!panel) return;
    panel->base.fill_width = fill_width;
    panel->base.fill_height = fill_height;
    zui_widget_invalidate(&panel->base);
}

void zui_panel_add_child(ZuiPanel *panel, ZuiWidget *child)
{
    if (!panel || !child) return;
    zui_widget_add_child(&panel->base, child);
}

void zui_panel_remove_child(ZuiPanel *panel, ZuiWidget *child)
{
    if (!panel || !child) return;
    zui_widget_remove_child(&panel->base, child);
}

void zui_panel_clear(ZuiPanel *panel)
{
    if (!panel) return;

    for (int i = panel->base.child_count - 1; i >= 0; i--) {
        zui_widget_destroy(panel->base.children[i]);
    }
    panel->base.child_count = 0;
}

ZuiWidget *zui_panel_as_widget(ZuiPanel *panel)
{
    return &panel->base;
}
