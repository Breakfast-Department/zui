<p align="right">
  English · <a href="README.id.md">Bahasa Indonesia</a>
</p>

<p align="center">
  <img src="assets/logo/zui-logo.png" alt="ZUI logo" width="180">
</p>

<h1 align="center">ZUI</h1>

<p align="center">
  An early-stage C11 GUI toolkit built directly for Wayland and rendered with EGL/OpenGL.
</p>

<p align="center">
  <a href="https://github.com/yusuf601/zui/actions/workflows/ci.yml"><img src="https://github.com/yusuf601/zui/actions/workflows/ci.yml/badge.svg" alt="CI status"></a>
  <img src="https://img.shields.io/badge/C-11-00599C.svg" alt="C11">
  <img src="https://img.shields.io/badge/platform-Wayland-6C4FBB.svg" alt="Wayland">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="MIT License"></a>
</p>

> [!WARNING]
> ZUI is under early development. Its API may change without notice. It currently targets Wayland only and does not yet provide an installation package, a stable external-consumption workflow, or an automated test suite.

- [Overview](#overview)
- [Priview](#preview)
- [Current Features](#current-features)
- [Project Status and Limitations](#project-status-and-limitations)
- [Requirements](#requirements)
- [Quick Start](#quick-start)
- [User Guide](#user-guide)
- [Public API Overview](#public-api-overview)
- [Developer Guide](#developer-guide)
- [Troubleshooting](#troubleshooting)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

## Overview

ZUI provides a small native GUI foundation for Linux applications running in a Wayland session. It connects to Wayland directly, creates EGL-backed windows, renders widgets through OpenGL, and exposes a compact C API.

The project is useful today for experimentation, learning, and contributing to a young toolkit. It is not yet intended as a production-ready or cross-platform GUI framework.

## Preview
<img src="./docs/screenshot/Screenshot_20260904_194437.png"/>

## Current Features

- Native Wayland windows using XDG Shell.
- EGL context and surface management.
- OpenGL rendering for windows, rectangles, text, icons, and images.
- Custom titlebars with built-in close, minimize, maximize, and hide buttons.
- Labels, clickable buttons, SVG icons, and raster images.
- Widget trees with padding, spacing, background colors, and rounded corners.
- Configurable widget cursors.
- Font loading from files or memory.
- GCC and Clang builds through CMake.

## Project Status and Limitations

ZUI currently has a deliberately small scope:

- Linux with a Wayland compositor is the only supported runtime.
- Public APIs are still evolving and may change between commits.
- The project builds a static `zui` target but has no CMake install/export rules.
- Runtime asset installation is not formalized; shaders, built-in icons, and logos must remain reachable.
- Automated tests and published releases are not available yet.
- X11, Windows, and macOS backends are not implemented.

## Requirements

You need:

- a Linux desktop running a Wayland session;
- a C11-capable compiler such as GCC or Clang;
- CMake 3.16 or newer;
- pkg-config;
- Wayland client, EGL, and cursor development files;
- EGL and OpenGL development files.

Install the build dependencies for your distribution.

### Debian / Ubuntu

```sh
sudo apt update
sudo apt install build-essential cmake pkg-config libwayland-dev libegl-dev libgl-dev
```

### Fedora

```sh
sudo dnf install gcc cmake pkgconf-pkg-config wayland-devel libglvnd-devel
```

### Arch Linux

```sh
sudo pacman -S --needed base-devel cmake pkgconf wayland libglvnd
```

Package names can differ on older distribution releases.

## Quick Start

Clone and build ZUI:

```sh
git clone https://github.com/yusuf601/zui.git
cd zui
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The `ZUI_BUILD_EXAMPLES` option is enabled by default. Run the included example from the repository root:

```sh
./build/zui_example
```

The program must run inside a usable Wayland session. The build also copies ZUI's bundled runtime assets into `build/assets/`.

## User Guide

### Application Lifecycle

A ZUI application follows seven steps:

1. Include `zui/zui.h`.
2. Call `zui_init()`.
3. Create a `ZuiWindow`.
4. Build its titlebar and content widget tree.
5. Show the window.
6. Poll events and render while the window is running.
7. Destroy the window and call `zui_shutdown()`.

Here is a minimal application using only the current public API:

```c
#include <zui/zui.h>
#include <stdio.h>

static void on_click(ZuiWidget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
    puts("Hello from ZUI!");
}

int main(void)
{
    if (!zui_init()) {
        return 1;
    }

    ZuiWindow *window = zui_window_create(800, 500, "My ZUI App");
    if (!window) {
        zui_shutdown();
        return 1;
    }

    ZuiButton *button = zui_button_create("Click me");
    zui_button_on_click(button, on_click, NULL);
    zui_widget_add_child(zui_window_content(window), (ZuiWidget *)button);

    zui_window_show(window);
    while (zui_window_running(window)) {
        zui_poll_events();
        zui_window_render(window);
    }

    zui_window_destroy(window);
    zui_shutdown();
    return 0;
}
```

See [`examples/main.c`](examples/main.c) for a fuller example with a titlebar, label, styled button, callback, and window controls.

Because ZUI does not yet provide install/export rules, the supported getting-started path is to build and experiment inside the repository. A stable system-wide integration procedure will be documented when packaging and runtime asset installation are available.

### Windows and Titlebars

Create a window with `zui_window_create(width, height, title)`. You can then set its title, corner radius, minimum size, maximum size, and logo.

Each window exposes:

- `zui_window_titlebar()` for the custom titlebar;
- `zui_window_content()` for the main content container.

The titlebar has start, center, and end regions. Add a widget with `zui_titlebar_set_start()`, `zui_titlebar_set_center()`, or `zui_titlebar_set_end()`. Built-in window control widgets are available through the close, minimize, maximize, and hide button functions.

### Widgets and Callbacks

Create text with `zui_label_create()` and interactive controls with `zui_button_create()`. A click callback receives the clicked `ZuiWidget *` and the `user_data` pointer registered with `zui_button_on_click()`.

Common widget operations include:

- showing or hiding a widget;
- setting a background color;
- setting a corner radius;
- selecting its cursor;
- attaching it to a parent.

Always check constructors that return pointers when a failed allocation or resource load must be handled.

### Layout and Styling

Add a child with `zui_widget_add_child(parent, child)`. The current window content container lays out its children vertically and centers them. Public layout controls currently expose padding and spacing; layout direction and alignment are not yet configurable through public setters.

Colors use normalized floating-point RGBA values. The helpers `ZUI_COLOR`, `ZUI_COLOR_RGB`, and `ZUI_COLOR_HEX` make common color declarations shorter.

### Fonts, Icons, and Images

- Load a font with `zui_font_load()` or `zui_font_load_memory()`, then assign it with `zui_label_set_font()`.
- Load SVG source data with `zui_icon_load_svg()` or `zui_icon_load_svg_data()`, then create a sized `ZuiIcon`.
- Create raster images from a file or memory with `zui_image_create()` and `zui_image_create_from_memory()`.

The icon and image implementations upload textures when used by the renderer. Resource creation can fail, so callers should check returned pointers.

### Ownership and Cleanup

Adding a widget to a parent makes it part of that widget tree. Destroying the parent recursively destroys attached child widgets, so do not destroy the same attached child separately.

Loaded fonts and `ZuiIconSource` objects have explicit destroy functions. Assigning a font to a label or creating an icon from an icon source does not transfer ownership of that standalone resource; keep it alive while it is in use and destroy it after the dependent widgets are gone.

Destroy top-level windows before `zui_shutdown()`.

### Runtime Assets

ZUI currently depends on bundled shader files and uses bundled icons and logos for its default window controls. CMake copies `assets/` into the build tree, while the runtime searches several development-oriented shader locations.

Until installation support is added, run examples from the repository root or the build directory and keep the copied asset tree beside the executable. Application-provided fonts, SVG files, and images must also remain readable at their supplied paths.

## Public API Overview

The public headers are the authoritative API reference.

| Area | Representative API | Header |
| --- | --- | --- |
| Lifecycle | `zui_init`, `zui_poll_events`, `zui_shutdown` | [`app.h`](include/zui/app.h) |
| Windows | `zui_window_create`, titlebar and window controls | [`window.h`](include/zui/window.h) |
| Widgets and cursors | labels, buttons, callbacks, `zui_widget_set_cursor` | [`widget.h`](include/zui/widget.h) |
| Layout | child attachment, padding, spacing | [`layout.h`](include/zui/layout.h) |
| Colors | `ZuiColor` and color macros | [`color.h`](include/zui/color.h) |
| Fonts | file/memory loading and text metrics | [`font.h`](include/zui/font.h) |
| SVG icons | source loading, sizing, tinting | [`icon.h`](include/zui/icon.h) |
| Images | file/memory loading, sizing, visibility | [`image.h`](include/zui/image.h) |

Include [`zui/zui.h`](include/zui/zui.h) when you want the complete public surface.

## Developer Guide

### Repository Layout

| Path | Responsibility |
| --- | --- |
| `include/zui/` | Public headers available to applications. |
| `include/zui/internal/` | Private interfaces shared by ZUI implementation units. |
| `src/core/` | Application state, windows, widgets, layout, fonts, icons, and images. |
| `src/platform/wayland/` | Wayland connection, input, windows, cursors, and generated protocols. |
| `src/renderer/opengl/` | EGL setup and OpenGL rendering. |
| `assets/` | Runtime shaders, bundled fonts, icons, and project logos. |
| `examples/` | Small programs exercising the public API. |
| `external/` | Vendored GLAD, NanoSVG, and stb sources. |

### Runtime Architecture

The principal runtime flow is:

```text
zui_init()
  -> connect to Wayland and discover globals
  -> initialize EGL
  -> create a window and its EGL surface
  -> initialize the OpenGL renderer on first render
  -> poll events, lay out widgets, and draw frames
  -> destroy windows and call zui_shutdown()
```

`src/core/app.c` owns global platform, EGL, and renderer state. The Wayland backend translates compositor and input events into window/widget operations. Window rendering computes layout, draws the widget tree, swaps the EGL buffer, and schedules the next frame.

### Development Builds

Create a Debug build with extra debug information and no optimization:

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```

Build only the libraries, without `zui_example`:

```sh
cmake -S . -B build-lib -DZUI_BUILD_EXAMPLES=OFF
cmake --build build-lib --parallel
```

The main targets are:

- `glad` — vendored OpenGL/EGL loader;
- `zui` — static ZUI library;
- `copy_assets` — copies bundled assets into the build tree;
- `zui_example` — example executable when examples are enabled.

### Coding Guidelines

- Use C11 and follow the formatting already present in nearby files.
- Keep application-facing declarations in `include/zui/` and implementation details in `include/zui/internal/`.
- Preserve the strict GCC/Clang warning configuration in `CMakeLists.txt`.
- Keep changes focused; avoid unrelated refactors.
- Check every allocation, platform object, and external resource at the boundary where it can fail.
- Keep public API additions small and document ownership and lifetime rules.
- Do not edit generated Wayland protocol files manually unless they are intentionally being regenerated.

### Verification and CI

Before opening a pull request, configure and build the affected combinations with GCC and Clang where available. At minimum, build the example and confirm there are no new compiler warnings.

The current CI workflow builds:

| Compiler | Configuration |
| --- | --- |
| GCC | Debug and Release |
| Clang | Debug and Release |

CI also invokes clang-tidy, but that step is currently non-blocking. ZUI has no automated test suite yet; do not treat a successful build as behavioral or visual validation.

## Troubleshooting

### CMake cannot find a dependency

Check the pkg-config modules first:

```sh
pkg-config --modversion wayland-client wayland-egl wayland-cursor egl
```

If a module is missing, install the corresponding development package from [Requirements](#requirements), then configure again.

### ZUI cannot connect to Wayland

Confirm that the process is running in a Wayland session:

```sh
echo "$XDG_SESSION_TYPE"
echo "$WAYLAND_DISPLAY"
```

`XDG_SESSION_TYPE` should normally report `wayland` and `WAYLAND_DISPLAY` should not be empty. A pure X11 session is not supported.

### EGL or OpenGL initialization fails

Confirm that your graphics driver provides EGL and OpenGL, then inspect the error printed by ZUI on standard error. Remote, nested, or headless sessions may not expose a usable EGL configuration.

### Shaders, icons, fonts, or images are missing

Run the example from the repository root with `./build/zui_example` or from its build tree, and confirm that `build/assets/` exists. For your own resources, verify that the supplied paths are readable from the application's runtime location.

## Roadmap

The current direction is intentionally broad:

- stabilize and document the public API;
- add CMake install/export support and packaging;
- introduce automated unit and integration tests;
- expand examples and reference documentation;
- explore additional platform backends after the Wayland backend matures.

No release dates are committed.

## Contributing

Focused issues and pull requests are welcome. Before contributing:

1. Read the [Developer Guide](#developer-guide).
2. Reproduce the behavior on Wayland.
3. Keep the change scoped and update both README languages when documentation changes.
4. Build with the relevant GCC and Clang configurations.
5. Describe what you verified and what could not be tested visually.

## License

ZUI is available under the [MIT License](LICENSE).

Copyright © 2026 Breakfast-Department.
