#include <zui/internal/widget_internal.h>
#include <stdlib.h>
#include <string.h>

ZuiWidget *zui_widget_create(size_t size, ZuiWidgetType type,
                              const ZuiWidgetVTable *vtable)
{
    ZuiWidget *widget = calloc(1, size);
    if (!widget) return NULL;

    widget->type = type;
    widget->vtable = vtable;
    widget->visible = true;
    widget->background = ZUI_COLOR(0, 0, 0, 0);
    widget->layout_dir = ZUI_LAYOUT_HORIZONTAL;
    widget->align = ZUI_ALIGN_START;

    return widget;
}

void zui_widget_destroy(ZuiWidget *widget)
{
    if (!widget) return;

    for (int i = 0; i < widget->child_count; i++) {
        zui_widget_destroy(widget->children[i]);
    }

    if (widget->vtable && widget->vtable->destroy) {
        widget->vtable->destroy(widget);
    }

    free(widget);
}

void zui_widget_add_child(ZuiWidget *parent, ZuiWidget *child)
{
    if (!parent || !child) return;
    if (parent->child_count >= ZUI_MAX_CHILDREN) return;

    child->parent = parent;
    parent->children[parent->child_count++] = child;
}

void zui_widget_remove_child(ZuiWidget *parent, ZuiWidget *child)
{
    if (!parent || !child) return;

    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            child->parent = NULL;
            for (int j = i; j < parent->child_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            return;
        }
    }
}

void zui_widget_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
    if (!widget || !widget->visible) return;

    if (widget->background.a > 0.0f) {
        ZuiRect rect = ZUI_RECT(widget->bounds.x, widget->bounds.y,
                                 widget->bounds.width, widget->bounds.height);
        if (widget->corner_radius > 0.0f) {
            switch (widget->corner_mode) {
            case ZUI_CORNERS_TOP:
                zui_renderer_draw_rounded_rect_top(renderer, rect,
                    widget->background, widget->corner_radius);
                break;
            case ZUI_CORNERS_BOTTOM:
                zui_renderer_draw_rounded_rect_bottom(renderer, rect,
                    widget->background, widget->corner_radius);
                break;
            case ZUI_CORNERS_NONE:
                zui_renderer_draw_rect(renderer, rect, widget->background);
                break;
            default:
                zui_renderer_draw_rounded_rect(renderer, rect,
                    widget->background, widget->corner_radius);
                break;
            }
        } else {
            zui_renderer_draw_rect(renderer, rect, widget->background);
        }
    }

    if (widget->vtable && widget->vtable->draw) {
        widget->vtable->draw(widget, renderer);
    }

    if (widget->child_count > 0) {
        zui_renderer_push_clip(renderer, ZUI_RECT(
            widget->bounds.x, widget->bounds.y,
            widget->bounds.width, widget->bounds.height),
            widget->corner_radius);

        for (int i = 0; i < widget->child_count; i++) {
            zui_widget_draw(widget->children[i], renderer);
        }

        zui_renderer_pop_clip(renderer);
    }
}

void zui_widget_layout(ZuiWidget *widget)
{
    if (!widget) return;

    if (widget->vtable && widget->vtable->layout) {
        widget->vtable->layout(widget);
    }

    for (int i = 0; i < widget->child_count; i++) {
        zui_widget_layout(widget->children[i]);
    }
}

bool zui_widget_contains_point(ZuiWidget *widget, float x, float y)
{
    if (!widget || !widget->visible) return false;
    return x >= widget->bounds.x &&
           x < widget->bounds.x + widget->bounds.width &&
           y >= widget->bounds.y &&
           y < widget->bounds.y + widget->bounds.height;
}

ZuiWidget *zui_widget_hit_test(ZuiWidget *widget, float x, float y)
{
    if (!widget || !widget->visible) return NULL;
    if (!zui_widget_contains_point(widget, x, y)) return NULL;

    for (int i = widget->child_count - 1; i >= 0; i--) {
        ZuiWidget *hit = zui_widget_hit_test(widget->children[i], x, y);
        if (hit) return hit;
    }

    if (widget->vtable && widget->vtable->hit_test) {
        if (widget->vtable->hit_test(widget, x, y)) {
            return widget;
        }
    }

    return widget;
}

void zui_widget_set_bounds(ZuiWidget *widget, float x, float y,
                            float width, float height)
{
    if (!widget) return;
    widget->bounds.x = x;
    widget->bounds.y = y;
    widget->bounds.width = width;
    widget->bounds.height = height;
}

void zui_widget_set_background(ZuiWidget *widget, ZuiColor color)
{
    if (!widget) return;
    widget->background = color;
}

void zui_widget_set_corner_radius(ZuiWidget *widget, float radius)
{
    if (!widget) return;
    widget->corner_radius = radius;
}

void zui_widget_set_padding(ZuiWidget *widget, float padding)
{
    if (!widget) return;
    widget->padding = padding;
}

void zui_widget_set_spacing(ZuiWidget *widget, float spacing)
{
    if (!widget) return;
    widget->spacing = spacing;
}
