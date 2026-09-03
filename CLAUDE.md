# CLAUDE.md

## Project Overview

**ZUI** is a lightweight, modern, and developer-friendly GUI Toolkit designed primarily for the **Wayland** ecosystem.

The project aims to provide a clean and flexible API for building graphical applications without forcing developers to deal directly with low-level Wayland and OpenGL/EGL implementation details.

ZUI is currently focused on Linux and Wayland, with a long-term goal of becoming a **cross-platform GUI toolkit**.

### Core Goals

ZUI should prioritize:

* **Developer Friendly** — Simple, intuitive, and consistent APIs.
* **Lightweight** — Minimal dependencies and low runtime overhead.
* **Modern** — Modern architecture, rendering, and API design.
* **Fast** — Efficient rendering and event handling.
* **Wayland Native** — First-class Wayland support.
* **Extensible** — Components and systems should be easy to extend.
* **Cross-Platform Ready** — Architecture should avoid unnecessary Wayland-specific coupling outside the platform layer.

---

## Technology Stack

### Language

Use **C** as the primary programming language unless there is a strong technical reason to introduce another language.

Prioritize:

* Clear ownership of resources.
* Explicit memory management.
* Predictable performance.
* Portable and maintainable code.
* Minimal abstractions where they do not provide meaningful value.

### Window System

Primary backend:

* **Wayland**

Wayland-specific code should be isolated inside a platform/backend layer whenever possible.

Do not allow Wayland implementation details to leak unnecessarily into the public ZUI API.

### Rendering

ZUI uses:

* **OpenGL**
* **EGL**

EGL is responsible for creating and managing the OpenGL rendering context and connecting OpenGL with the native Wayland surface.

The rendering architecture should be designed so that the renderer can eventually be replaced or extended with other graphics backends without rewriting the entire toolkit.

Potential future backends may include:

* Vulkan
* Software rendering
* Other platform-specific rendering APIs

Do not implement these backends prematurely.

---

## Architecture

Prefer a modular architecture similar to:

```text
zui/
├── include/
│   └── zui/
│       ├── zui.h
│       ├── window.h
│       ├── widget.h
│       ├── input.h
│       └── ...
│
├── src/
│   ├── core/
│   ├── platform/
│   │   └── wayland/
│   ├── renderer/
│   │   └── opengl/
│   ├── widgets/
│   └── input/
│
├── examples/
├── tests/
├── docs/
├── CMakeLists.txt
└── README.md
```

The exact structure may evolve as the project grows.

### Separation of Concerns

Keep these systems separate:

```text
Application
    ↓
ZUI Public API
    ↓
Core / Widget System
    ↓
Platform Layer      Renderer
    ↓                  ↓
Wayland             OpenGL/EGL
```

The core should not directly depend on Wayland implementation details.

---

## Public API Design

The public API is one of the most important parts of ZUI.

APIs should be:

* Simple.
* Predictable.
* Consistent.
* Explicit.
* Easy to discover.
* Easy to document.

Prefer APIs that can be understood without reading the implementation.

For example:

```c
ZuiWindow *window = zui_window_create(800, 600, "My Application");

zui_window_show(window);

while (zui_window_running(window)) {
    zui_poll_events();
    zui_begin_frame();

    /* UI */

    zui_end_frame();
}

zui_window_destroy(window);
```

Avoid unnecessarily complicated initialization or configuration.

---

## Rendering Principles

ZUI should use a retained or hybrid UI architecture where appropriate, while keeping rendering efficient.

Rendering should:

* Minimize unnecessary draw calls.
* Batch compatible geometry where possible.
* Avoid unnecessary allocations per frame.
* Avoid unnecessary GPU synchronization.
* Reuse GPU resources.
* Keep frame rendering predictable.

The renderer should be independent from widgets as much as practical.

Widgets describe **what** should be rendered.

The renderer decides **how** it is rendered.

---

## Wayland Principles

Wayland is the primary platform backend.

The implementation should correctly handle:

* Wayland display connection.
* Registry discovery.
* Compositor.
* Shared memory where required.
* XDG Shell.
* XDG Surface.
* Toplevel surfaces.
* Seat.
* Keyboard input.
* Pointer input.
* Window configuration.
* Frame callbacks.
* Surface lifecycle.

Do not assume X11 functionality when implementing the Wayland backend.

Avoid unnecessary dependencies on XWayland.

---

## EGL + OpenGL

EGL should be responsible for:

1. Connecting OpenGL to the Wayland display.
2. Selecting an appropriate EGL configuration.
3. Creating the EGL context.
4. Creating the EGL window surface.
5. Managing buffer swapping.
6. Cleaning up EGL resources.

OpenGL should be used for actual UI rendering.

The rendering initialization should be isolated from the widget system.

Conceptually:

```text
Wayland Display
      ↓
     EGL
      ↓
 OpenGL Context
      ↓
 ZUI Renderer
      ↓
     Widgets
```

Avoid relying on deprecated OpenGL functionality where practical.

Prefer modern OpenGL techniques and programmable shaders.

---

## Event System

ZUI should provide a unified event system.

Platform-specific events should be translated into ZUI events before reaching application code.

For example:

```text
Wayland Pointer Event
        ↓
Wayland Backend
        ↓
ZUI Input Event
        ↓
Widget
```

Application developers should not need to understand Wayland protocol events to handle normal UI interactions.

---

## Widget System

Widgets should be modular and composable.

Potential core widgets include:

* Window
* Container
* Button
* Label
* Text Input
* Checkbox
* Slider
* Scroll View
* Image
* Menu

Do not implement a large widget library before the underlying architecture is stable.

Prioritize a small set of high-quality primitives first.

---

## Memory Management

Memory usage should be predictable.

Avoid:

* Unnecessary heap allocations.
* Per-frame allocations.
* Hidden global state.
* Excessive reference counting.
* Unnecessary object duplication.

Every allocated resource should have a clear owner and destruction path.

GPU and platform resources must also be explicitly released.

---

## Dependencies

Keep dependencies minimal.

Prefer system libraries where appropriate:

* Wayland
* EGL
* OpenGL

Additional dependencies should only be introduced when they provide significant value.

Do not add a dependency simply to avoid implementing a small amount of functionality internally.

---

## Error Handling

Errors should be explicit and useful.

Avoid silently ignoring failures.

For recoverable errors:

```c
ZuiResult result = zui_window_create(...);
```

For fatal initialization errors, provide useful diagnostic messages.

Error messages should identify:

* What failed.
* Which subsystem failed.
* Why it failed when the underlying API provides that information.

---

## Performance

Performance is an important design constraint, but **do not sacrifice API usability for premature optimization**.

Optimize based on measurable bottlenecks.

Important areas:

* Frame rendering.
* Event processing.
* Layout calculations.
* Text rendering.
* Memory allocation.
* GPU resource management.
* Input latency.

Avoid micro-optimizations that significantly complicate the codebase without measurable benefit.

---

## Developer Experience

ZUI should feel pleasant to use.

Documentation and examples should favor simple, readable code.

A basic application should require as little boilerplate as reasonably possible.

Example:

```c
#include <zui/zui.h>

int main(void)
{
    ZuiWindow *window =
        zui_window_create(800, 600, "Hello ZUI");

    zui_window_show(window);

    while (zui_window_running(window)) {
        zui_poll_events();

        zui_begin_frame();

        zui_button("Hello World");

        zui_end_frame();
    }

    zui_window_destroy(window);

    return 0;
}
```

The API may change during early development, but the principle should remain:

> **Simple things should be simple.**

---

## Cross-Platform Strategy

Wayland is the first-class backend, but the architecture must not make future platforms impossible.

Platform-specific functionality should live behind interfaces such as:

```text
platform/
├── wayland/
├── x11/        # Future
├── windows/    # Future
└── macos/      # Future
```

Do not create fake cross-platform abstractions before there is a real need for them.

Build abstractions around stable concepts rather than around assumptions about future platforms.

---

## Build System

Use **CMake** as the primary build system.

The project should support:

```bash
cmake -B build
cmake --build build
```

Development builds should provide useful compiler warnings.

Prefer strict compilation settings where practical.

---

## Code Style

Prioritize readability over cleverness.

Use:

* `snake_case` for functions and variables.
* `PascalCase` or project-consistent naming for public types.
* Clear and descriptive names.
* Small functions.
* Explicit ownership.
* Minimal global state.

Avoid:

* Extremely long functions.
* Deeply nested control flow.
* Clever macros.
* Unnecessary abstractions.
* Hidden side effects.

Comments should explain **why**, not simply repeat what the code does.

---

## Git Commit Convention

Use conventional commit-style messages.

Examples:

```text
feat: add wayland window backend
feat: implement egl context creation
feat: add basic widget system
fix: handle xdg surface configuration
refactor: separate renderer from platform backend
docs: improve getting started guide
test: add window lifecycle tests
```

Keep commits focused and logically separated.

---

## Development Priorities

Development should generally follow this order:

### Phase 1 — Foundation

* Project structure.
* CMake setup.
* Public API foundation.
* Wayland connection.
* Basic window creation.

### Phase 2 — Rendering

* EGL initialization.
* OpenGL context.
* Swapchain/surface presentation.
* Basic shader pipeline.
* Basic rectangle rendering.

### Phase 3 — Input

* Pointer.
* Keyboard.
* Window events.
* Unified event system.

### Phase 4 — UI

* Widget tree.
* Layout system.
* Basic widgets.
* Styling system.

### Phase 5 — Advanced Features

* Text rendering.
* Images.
* Animations.
* Accessibility.
* Clipboard.
* Drag and drop.

### Phase 6 — Portability

After the core architecture is stable:

* X11 backend.
* Windows backend.
* macOS backend.
* Additional rendering backends where appropriate.

---

## Design Philosophy

ZUI should follow these principles:

> **Lightweight by default.**

> **Simple APIs, powerful internals.**

> **Native where it matters.**

> **Portable where it makes sense.**

> **Performance without unnecessary complexity.**

The goal is not to create the largest GUI framework.

The goal is to create a **small, modern, performant, and enjoyable GUI toolkit** that developers can understand and build upon.
