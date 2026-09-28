# Event Handling

ZUI provides a callback-based event system for handling user interactions.

## Event Types

### Mouse Events

| Event | Description |
|-------|-------------|
| Click | Mouse button released on widget |
| Mouse Enter | Cursor enters widget bounds |
| Mouse Leave | Cursor exits widget bounds |
| Mouse Down | Mouse button pressed |
| Mouse Up | Mouse button released |
| Mouse Move | Cursor moved over widget |
| Scroll | Mouse wheel scrolled |

### Keyboard Events

| Event | Description |
|-------|-------------|
| Key Press | Key pressed down |
| Key Release | Key released |
| Text Input | Character typed |

### Widget Events

| Event | Description |
|-------|-------------|
| Focus | Widget gained/lost focus |
| Value Change | Widget value changed (sliders, inputs) |

## Registering Callbacks

### Button Click

```c
void on_click(ZuiWidget *widget, void *data)
{
    printf("Button clicked!\n");
}

ZuiWidget *button = zui_button_new("Click Me");
zui_button_on_click(ZUI_BUTTON(button), on_click, NULL);
```

### With User Data

```c
typedef struct {
    int counter;
    ZuiWidget *label;
} AppState;

void on_click(ZuiWidget *widget, void *data)
{
    AppState *state = (AppState *)data;
    state->counter++;
    
    char buf[32];
    snprintf(buf, sizeof(buf), "Count: %d", state->counter);
    zui_label_set_text(ZUI_LABEL(state->label), buf);
}

// Usage
AppState state = { .counter = 0 };
state.label = zui_label_create("Count: 0");

ZuiWidget *button = zui_button_new("Increment");
zui_button_on_click(button, on_click, &state);
```

### Value Change Events

```c
// Slider
void on_slider_change(ZuiSlider *slider, float value, void *data)
{
    printf("Slider value: %.2f\n", value);
}

zui_slider_on_change(ZUI_SLIDER(slider), on_slider_change, NULL);

// Text Input
void on_text_change(ZuiTextInput *input, const char *text, void *data)
{
    printf("Text: %s\n", text);
}

zui_textinput_on_change(ZUI_TEXTINPUT(input), on_text_change, NULL);

// Checkbox
void on_checkbox_change(ZuiCheckbox *cb, bool checked, void *data)
{
    printf("Checked: %s\n", checked ? "yes" : "no");
}

zui_checkbox_on_change(ZUI_CHECKBOX(checkbox), on_checkbox_change, NULL);

// Dropdown
void on_dropdown_change(ZuiDropdown *dd, int index, const char *item, void *data)
{
    printf("Selected: %s (index %d)\n", item, index);
}

zui_dropdown_on_change(ZUI_DROPDOWN(dropdown), on_dropdown_change, NULL);
```

## Mouse Button Constants

```c
// In click handlers
void on_click(ZuiWidget *widget, uint32_t button, void *data)
{
    switch (button) {
        case BTN_LEFT:
            printf("Left click\n");
            break;
        case BTN_RIGHT:
            printf("Right click\n");
            break;
        case BTN_MIDDLE:
            printf("Middle click\n");
            break;
    }
}
```

## Keyboard Input

### Handling Key Events

```c
void on_key(ZuiWidget *widget, uint32_t key, uint32_t sym, 
            const char *text, bool pressed)
{
    if (pressed) {
        printf("Key pressed: sym=%u\n", sym);
        
        // Check for specific keys
        if (sym == XKB_KEY_Escape) {
            printf("Escape pressed!\n");
        }
    }
}
```

### Common Key Symbols

```c
#include <xkbcommon/xkbcommon-keysyms.h>

// Arrow keys
XKB_KEY_Left, XKB_KEY_Right, XKB_KEY_Up, XKB_KEY_Down

// Modifiers
XKB_KEY_Shift_L, XKB_KEY_Control_L, XKB_KEY_Alt_L

// Common keys
XKB_KEY_Return, XKB_KEY_Escape, XKB_KEY_BackSpace
XKB_KEY_Tab, XKB_KEY_space
XKB_KEY_Home, XKB_KEY_End, XKB_KEY_Delete

// Function keys
XKB_KEY_F1, XKB_KEY_F2, ... XKB_KEY_F12
```

## Focus Management

### Setting Focus

```c
// Focus a widget
zui_widget_focus(input);

// Remove focus
zui_widget_unfocus(input);
```

### Focus Order

Focus moves through widgets in the order they were added. Use Tab to move forward.

## Event Patterns

### Toggle Button

```c
typedef struct {
    bool active;
    ZuiWidget *button;
} ToggleState;

void on_toggle(ZuiWidget *widget, void *data)
{
    ToggleState *state = (ToggleState *)data;
    state->active = !state->active;
    
    if (state->active) {
        zui_button_set_colors(ZUI_BUTTON(state->button),
            ZUI_COLOR_HEX(0x2ecc71),
            ZUI_COLOR_HEX(0x58d68d),
            ZUI_COLOR_HEX(0x27ae60));
        zui_button_set_text(ZUI_BUTTON(state->button), "ON");
    } else {
        zui_button_set_colors(ZUI_BUTTON(state->button),
            ZUI_COLOR_HEX(0x7f8c8d),
            ZUI_COLOR_HEX(0x95a5a6),
            ZUI_COLOR_HEX(0x6c7a7d));
        zui_button_set_text(ZUI_BUTTON(state->button), "OFF");
    }
}
```

### Form Validation

```c
typedef struct {
    ZuiWidget *name_input;
    ZuiWidget *email_input;
    ZuiWidget *submit_button;
    ZuiWidget *error_label;
} Form;

void validate_form(Form *form)
{
    const char *name = zui_textinput_get_text(ZUI_TEXTINPUT(form->name_input));
    const char *email = zui_textinput_get_text(ZUI_TEXTINPUT(form->email_input));
    
    bool valid = (strlen(name) > 0 && strchr(email, '@') != NULL);
    
    // Enable/disable submit button
    if (valid) {
        zui_widget_set_visible(form->error_label, false);
        zui_button_set_colors(ZUI_BUTTON(form->submit_button),
            ZUI_COLOR_HEX(0x3498db), /* ... */);
    } else {
        zui_widget_set_visible(form->error_label, true);
        zui_button_set_colors(ZUI_BUTTON(form->submit_button),
            ZUI_COLOR_HEX(0x7f8c8d), /* ... */);
    }
}

void on_input_change(ZuiTextInput *input, const char *text, void *data)
{
    Form *form = (Form *)data;
    validate_form(form);
}
```

### Menu Selection

```c
void on_menu_select(ZuiMenuItem *item, void *data)
{
    const char *action = (const char *)data;
    
    if (strcmp(action, "new") == 0) {
        create_new_document();
    } else if (strcmp(action, "open") == 0) {
        open_document();
    } else if (strcmp(action, "save") == 0) {
        save_document();
    }
}

// Setup
zui_menuitem_on_click(new_item, on_menu_select, "new");
zui_menuitem_on_click(open_item, on_menu_select, "open");
zui_menuitem_on_click(save_item, on_menu_select, "save");
```

### Drag Operations

```c
typedef struct {
    bool dragging;
    float start_x, start_y;
    float offset_x, offset_y;
} DragState;

void on_mouse_down(ZuiWidget *widget, float x, float y, uint32_t button, void *data)
{
    DragState *ds = (DragState *)data;
    if (button == BTN_LEFT) {
        ds->dragging = true;
        ds->start_x = x;
        ds->start_y = y;
    }
}

void on_mouse_move(ZuiWidget *widget, float x, float y, void *data)
{
    DragState *ds = (DragState *)data;
    if (ds->dragging) {
        ds->offset_x = x - ds->start_x;
        ds->offset_y = y - ds->start_y;
        // Update widget position
    }
}

void on_mouse_up(ZuiWidget *widget, float x, float y, uint32_t button, void *data)
{
    DragState *ds = (DragState *)data;
    if (button == BTN_LEFT) {
        ds->dragging = false;
    }
}
```

## Cursor Management

```c
// Set cursor for interactive widgets
zui_set_cursor(button, ZUI_CURSOR_POINTER);
zui_set_cursor(resize_handle, ZUI_CURSOR_RESIZE_EW);
zui_set_cursor(text_input, ZUI_CURSOR_TEXT);
```

### Cursor Types

| Cursor | Use Case |
|--------|----------|
| `ZUI_CURSOR_DEFAULT` | Normal arrow |
| `ZUI_CURSOR_POINTER` | Clickable items |
| `ZUI_CURSOR_TEXT` | Text input |
| `ZUI_CURSOR_CROSSHAIR` | Precise selection |
| `ZUI_CURSOR_MOVE` | Movable items |
| `ZUI_CURSOR_RESIZE_EW` | Horizontal resize |
| `ZUI_CURSOR_RESIZE_NS` | Vertical resize |
| `ZUI_CURSOR_RESIZE_NWSE` | Diagonal resize |

## Event Flow

1. **Capture Phase**: Events travel from root to target (not exposed in ZUI)
2. **Target Phase**: Event reaches target widget
3. **Bubble Phase**: Events can bubble up to parent (automatic for some events)

```
Window
└── Panel
    └── Button  ← Click event fires here first
        
Event handler called on Button
```

## Best Practices

1. **Keep handlers simple** — Do one thing per handler
2. **Use user data** — Pass state through the data parameter
3. **Validate early** — Check conditions before processing
4. **Update UI immediately** — Users expect instant feedback
5. **Don't block** — Long operations should be async

---

**See Also:** [Widget Overview](../widgets/overview.md) · [Window Management](windows.md)
