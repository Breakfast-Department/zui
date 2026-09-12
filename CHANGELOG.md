# Changelog

All notable changes to ZUI will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Path API for custom vector shapes (`zui_path_create`, bezier curves, arcs)
- Shape widget with fill, stroke, children, and event handling
- Stencil-based path clipping for shape children
- CMake install/export rules and pkg-config support
- Version API (`zui_version()`, `zui_version_info()`)
- `zui_widget_set_position()` for manual widget positioning

### Changed
- Improved curve tessellation (32 segments for smoother beziers)
- Extracted `ZuiCursor` enum to separate header

### Fixed
- Double-free in shape widget destruction
- Header guard style consistency

## [0.1.0] - 2024-XX-XX

### Added
- Initial release
- Native Wayland window management (XDG Shell)
- EGL/OpenGL rendering backend
- Core widgets: Button, Label, Panel, Checkbox, Slider, TextInput, Dropdown
- Advanced widgets: ScrollView, GridView, SplitView, ProgressBar, CircularProgress
- Chart widgets: PieChart, BarChart, LineChart
- Menu system: MenuBar, Menu, MenuItem
- Custom window decorations with minimize/maximize/close buttons
- Font loading and text rendering
- SVG icon support
- Image loading (PNG, JPEG, WebP)
- Video playback (MPV backend)
- Audio playback (miniaudio)
- Flexible layout system (horizontal/vertical, alignment, padding, spacing)
- Mouse and keyboard input handling
- Cursor management
- Resource embedding system

[Unreleased]: https://github.com/user/zui/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/user/zui/releases/tag/v0.1.0
