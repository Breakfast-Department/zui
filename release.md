# ZUI v0.1.0

**Lightweight modern GUI toolkit for Wayland**

This is the first release of ZUI, a developer-friendly C11 GUI toolkit designed for the Wayland ecosystem.

## Highlights

- Native Wayland support with XDG Shell
- Hardware-accelerated rendering via EGL/OpenGL
- Flexible widget system with custom styling
- Built-in window decorations
- Resource embedding for self-contained applications

## Features

### Core
- Window management (create, resize, minimize, maximize, fullscreen)
- Custom window decorations with drag-to-move support
- Mouse and keyboard input handling
- Cursor management

### Widgets
- **Basic:** Button, Label, Panel, Checkbox, Slider, TextInput, Dropdown
- **Layout:** ScrollView, GridView, SplitView
- **Progress:** ProgressBar, CircularProgress
- **Charts:** PieChart, BarChart, LineChart
- **Menu:** MenuBar, Menu, MenuItem

### Graphics
- Path API for custom vector shapes (bezier curves, arcs, polygons)
- Shape widget with fill, stroke, and stencil-based clipping
- Font loading and text rendering
- SVG icon support
- Image loading (PNG, JPEG, WebP)

### Media
- Video playback (MPV backend, optional)
- Audio playback (miniaudio)

### Developer Experience
- Simple, intuitive C API
- CMake build system with install/export rules
- pkg-config support
- Resource embedding system
- Version API (`zui_version()`)

## Installation

### From Source

```bash
git clone https://github.com/ezravln/zui.git
cd zui
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

### Dependencies

- Wayland (wayland-client, wayland-egl)
- EGL
- OpenGL
- libxkbcommon
- libwebp
- libmpv (optional, for video playback)

## Quick Start

```c
#include <zui/zui.h>

int main(void)
{
    zui_init();
    
    ZuiWindow *window = zui_window_create(800, 600, "Hello ZUI");
    zui_window_show(window);
    
    while (zui_window_running(window)) {
        zui_window_render(window);
        zui_poll_events();
    }
    
    zui_window_destroy(window);
    zui_shutdown();
    
    return 0;
}
```

## Links

- [Documentation](https://github.com/ezravln/zui/tree/main/docs)
- [Examples](https://github.com/ezravln/zui/tree/main/examples)
- [CHANGELOG](https://github.com/ezravln/zui/blob/main/CHANGELOG.md)

## License

MIT
