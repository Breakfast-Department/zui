# Layout System

ZUI uses a flexbox-inspired layout system based on panels. This guide covers how to create responsive, well-organized UIs.

## Basic Concepts

### Panels as Layout Containers

Panels are the primary layout containers in ZUI. They arrange their children either vertically or horizontally.

```c
ZuiPanel *panel = zui_panel_create();
zui_panel_set_layout(panel, ZUI_LAYOUT_VERTICAL);
```

### Layout Direction

| Direction | Description |
|-----------|-------------|
| `ZUI_LAYOUT_VERTICAL` | Stack children top to bottom |
| `ZUI_LAYOUT_HORIZONTAL` | Stack children left to right |

## Spacing and Padding

### Spacing

Space between children:

```c
zui_panel_set_spacing(ZUI_PANEL(panel), 12);  // 12px between children
```

```
┌────────────────┐
│ ┌────────────┐ │
│ │  Child 1   │ │
│ └────────────┘ │
│    ↕ 12px      │
│ ┌────────────┐ │
│ │  Child 2   │ │
│ └────────────┘ │
└────────────────┘
```

### Padding

Space around the content:

```c
// top, right, bottom, left
zui_panel_set_padding(ZUI_PANEL(panel), 20, 16, 20, 16);
```

```
┌──────────────────────┐
│  ↑ 20px              │
│ ←16px→┌────────┐←16px→│
│       │Content │     │
│       └────────┘     │
│  ↓ 20px              │
└──────────────────────┘
```

## Alignment

### Main Axis Alignment

Controls where children are placed along the layout direction:

```c
zui_panel_set_alignment(ZUI_PANEL(panel), ZUI_ALIGN_CENTER, ZUI_ALIGN_CENTER);
//                                        ↑ main axis     ↑ cross axis
```

For a **vertical** panel:
- `ZUI_ALIGN_START` — Top
- `ZUI_ALIGN_CENTER` — Middle
- `ZUI_ALIGN_END` — Bottom

For a **horizontal** panel:
- `ZUI_ALIGN_START` — Left
- `ZUI_ALIGN_CENTER` — Center
- `ZUI_ALIGN_END` — Right

### Cross Axis Alignment

Controls alignment perpendicular to the layout direction.

### Visual Examples

```
Vertical, Main: START          Vertical, Main: CENTER
┌──────────────────┐           ┌──────────────────┐
│ ┌──────────┐     │           │                  │
│ │ Child 1  │     │           │ ┌──────────┐     │
│ └──────────┘     │           │ │ Child 1  │     │
│ ┌──────────┐     │           │ └──────────┘     │
│ │ Child 2  │     │           │ ┌──────────┐     │
│ └──────────┘     │           │ │ Child 2  │     │
│                  │           │ └──────────┘     │
└──────────────────┘           └──────────────────┘

Horizontal, Cross: CENTER
┌──────────────────────────────┐
│          ┌──────┐ ┌──────┐   │
│          │  C1  │ │  C2  │   │
│          └──────┘ └──────┘   │
└──────────────────────────────┘
```

## Sizing

### Fixed Size

```c
// Set exact dimensions
zui_widget_set_size(widget, 200, 100);  // 200x100 pixels
zui_panel_set_size(ZUI_PANEL(panel), 300, 200);
```

### Fill Parent

Make a widget expand to fill available space:

```c
// Fill width only
zui_widget_set_fill(widget, true, false);
zui_panel_set_fill(ZUI_PANEL(panel), true, false);

// Fill both dimensions
zui_widget_set_fill(widget, true, true);
```

### Combining Fixed and Fill

```c
// Sidebar: fixed width, fill height
zui_widget_set_size(sidebar, 200, 0);
zui_widget_set_fill(sidebar, false, true);

// Content: fill both
zui_widget_set_fill(content, true, true);
```

## Common Layout Patterns

### Centered Content

```c
ZuiWidget *create_centered_layout(ZuiWidget *content)
{
    ZuiWidget *container = zui_panel_new();
    zui_panel_set_layout(ZUI_PANEL(container), ZUI_LAYOUT_VERTICAL);
    zui_panel_set_alignment(ZUI_PANEL(container), ZUI_ALIGN_CENTER, ZUI_ALIGN_CENTER);
    zui_panel_set_fill(ZUI_PANEL(container), true, true);
    
    zui_panel_add_child(ZUI_PANEL(container), content);
    
    return container;
}
```

### Sidebar Layout

```c
ZuiWidget *create_sidebar_layout(ZuiWidget *sidebar, ZuiWidget *main)
{
    ZuiWidget *container = zui_panel_new();
    zui_panel_set_layout(ZUI_PANEL(container), ZUI_LAYOUT_HORIZONTAL);
    zui_panel_set_fill(ZUI_PANEL(container), true, true);
    
    // Sidebar: fixed width
    zui_widget_set_size(sidebar, 250, 0);
    zui_widget_set_fill(sidebar, false, true);
    
    // Main: fill remaining space
    zui_widget_set_fill(main, true, true);
    
    zui_panel_add_child(ZUI_PANEL(container), sidebar);
    zui_panel_add_child(ZUI_PANEL(container), main);
    
    return container;
}
```

```
┌─────────┬───────────────────────────┐
│         │                           │
│ Sidebar │         Main              │
│  250px  │        (fill)             │
│         │                           │
└─────────┴───────────────────────────┘
```

### Header-Content-Footer

```c
ZuiWidget *create_page_layout(ZuiWidget *header, ZuiWidget *content, 
                               ZuiWidget *footer)
{
    ZuiWidget *container = zui_panel_new();
    zui_panel_set_layout(ZUI_PANEL(container), ZUI_LAYOUT_VERTICAL);
    zui_panel_set_fill(ZUI_PANEL(container), true, true);
    
    // Header: fixed height
    zui_widget_set_size(header, 0, 60);
    zui_widget_set_fill(header, true, false);
    
    // Content: fill remaining
    zui_widget_set_fill(content, true, true);
    
    // Footer: fixed height
    zui_widget_set_size(footer, 0, 40);
    zui_widget_set_fill(footer, true, false);
    
    zui_panel_add_child(ZUI_PANEL(container), header);
    zui_panel_add_child(ZUI_PANEL(container), content);
    zui_panel_add_child(ZUI_PANEL(container), footer);
    
    return container;
}
```

```
┌───────────────────────────────────┐
│              Header (60px)        │
├───────────────────────────────────┤
│                                   │
│              Content              │
│              (fill)               │
│                                   │
├───────────────────────────────────┤
│              Footer (40px)        │
└───────────────────────────────────┘
```

### Card Grid

Using GridView for a grid of cards:

```c
ZuiWidget *create_card_grid(void)
{
    ZuiWidget *grid = zui_gridview_new(3);  // 3 columns
    zui_gridview_set_gap(ZUI_GRIDVIEW(grid), 16, 16);
    zui_gridview_set_padding(ZUI_GRIDVIEW(grid), 20);
    zui_widget_set_fill(grid, true, true);
    
    // Add cards
    for (int i = 0; i < 9; i++) {
        char title[32];
        snprintf(title, sizeof(title), "Card %d", i + 1);
        ZuiWidget *card = create_card(title);
        zui_gridview_add_child(ZUI_GRIDVIEW(grid), card);
    }
    
    return grid;
}
```

```
┌────────┬────────┬────────┐
│ Card 1 │ Card 2 │ Card 3 │
├────────┼────────┼────────┤
│ Card 4 │ Card 5 │ Card 6 │
├────────┼────────┼────────┤
│ Card 7 │ Card 8 │ Card 9 │
└────────┴────────┴────────┘
```

### Form Layout

```c
ZuiWidget *create_form_field(const char *label, ZuiWidget *input)
{
    ZuiWidget *row = zui_panel_new();
    zui_panel_set_layout(ZUI_PANEL(row), ZUI_LAYOUT_HORIZONTAL);
    zui_panel_set_spacing(ZUI_PANEL(row), 12);
    zui_panel_set_alignment(ZUI_PANEL(row), ZUI_ALIGN_START, ZUI_ALIGN_CENTER);
    zui_panel_set_fill(ZUI_PANEL(row), true, false);
    
    // Label: fixed width for alignment
    ZuiWidget *lbl = zui_label_new(label);
    zui_widget_set_size(lbl, 100, 0);
    
    // Input: fill remaining
    zui_widget_set_fill(input, true, false);
    
    zui_panel_add_child(ZUI_PANEL(row), lbl);
    zui_panel_add_child(ZUI_PANEL(row), input);
    
    return row;
}

ZuiWidget *create_form(void)
{
    ZuiWidget *form = zui_panel_new();
    zui_panel_set_layout(ZUI_PANEL(form), ZUI_LAYOUT_VERTICAL);
    zui_panel_set_spacing(ZUI_PANEL(form), 12);
    zui_panel_set_padding(ZUI_PANEL(form), 20, 20, 20, 20);
    
    zui_panel_add_child(ZUI_PANEL(form),
        create_form_field("Name:", zui_textinput_new("Enter name")));
    zui_panel_add_child(ZUI_PANEL(form),
        create_form_field("Email:", zui_textinput_new("Enter email")));
    zui_panel_add_child(ZUI_PANEL(form),
        create_form_field("Phone:", zui_textinput_new("Enter phone")));
    
    return form;
}
```

```
┌────────────────────────────────┐
│ Name:    ┌──────────────────┐  │
│          │                  │  │
│          └──────────────────┘  │
│                                │
│ Email:   ┌──────────────────┐  │
│          │                  │  │
│          └──────────────────┘  │
│                                │
│ Phone:   ┌──────────────────┐  │
│          │                  │  │
│          └──────────────────┘  │
└────────────────────────────────┘
```

## Responsive Design

### Dynamic Layout Updates

```c
void update_layout_for_size(ZuiWidget *container, int width, int height)
{
    if (width < 600) {
        // Mobile: stack vertically
        zui_panel_set_layout(ZUI_PANEL(container), ZUI_LAYOUT_VERTICAL);
    } else {
        // Desktop: horizontal
        zui_panel_set_layout(ZUI_PANEL(container), ZUI_LAYOUT_HORIZONTAL);
    }
}

// In main loop
int w, h;
zui_window_get_size(window, &w, &h);
update_layout_for_size(main_panel, w, h);
```

### Scaling Widgets

```c
void scale_ui(ZuiWidget *root, float scale)
{
    // Scale padding
    float base_padding = 16;
    zui_panel_set_padding(ZUI_PANEL(root), 
        base_padding * scale, base_padding * scale,
        base_padding * scale, base_padding * scale);
    
    // Scale spacing
    zui_panel_set_spacing(ZUI_PANEL(root), 12 * scale);
}
```

## SplitView

For resizable split panes:

```c
ZuiWidget *split = zui_splitview_new(false);  // Horizontal split

ZuiWidget *left = zui_panel_new();
ZuiWidget *right = zui_panel_new();

zui_splitview_add_child(ZUI_SPLITVIEW(split), left);
zui_splitview_add_child(ZUI_SPLITVIEW(split), right);

// Set initial split position (0.0 - 1.0)
zui_splitview_set_position(ZUI_SPLITVIEW(split), 0, 0.3);  // 30% left
```

## ScrollView

For scrollable content:

```c
ZuiWidget *scroll = zui_scrollview_new();

ZuiWidget *content = zui_panel_new();
// ... add many children to content ...

zui_scrollview_set_content(ZUI_SCROLLVIEW(scroll), content);
zui_widget_set_fill(scroll, true, true);
```

## Best Practices

1. **Use panels for structure** — Panels are cheap and help organize your layout
2. **Nest panels freely** — Complex layouts come from simple nested panels
3. **Set fill strategically** — Not everything should fill; use fixed sizes for sidebars, headers
4. **Test different sizes** — Resize your window to verify layouts work at all sizes
5. **Use consistent spacing** — Pick 2-3 spacing values and use them throughout

---

**See Also:** [Panel](../widgets/panel.md) · [GridView](../widgets/gridview.md) · [SplitView](../widgets/splitview.md)
