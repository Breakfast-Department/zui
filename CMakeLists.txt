cmake_minimum_required(VERSION 3.16)

project(zui
  VERSION 0.1.0
  DESCRIPTION "Lightweight modern GUI toolkit for Wayland"
  LANGUAGES C
)

# Extract version components
set(ZUI_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(ZUI_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(ZUI_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# Generate version header
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/zui/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/zui/version.h"
  @ONLY
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Strict compiler warnings for development
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -Wdouble-promotion
    -Wformat=2
    -Wno-unused-parameter
  )
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g3 -O0)
  endif()
endif()

# Find Wayland
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND REQUIRED
  wayland-client
  wayland-egl
  wayland-cursor
)

# Find EGL and OpenGL
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(XKB REQUIRED xkbcommon)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# Optional: libmpv for video playback
pkg_check_modules(MPV QUIET mpv)
option(ZUI_VIDEO_MPV "Build libmpv video backend" ${MPV_FOUND})

# WebP support
pkg_check_modules(WEBP REQUIRED libwebp)

# GLAD (vendored OpenGL/EGL loader)
add_library(glad STATIC
  external/glad/src/glad.c
  external/glad/src/egl.c
)
target_include_directories(glad PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# Suppress warnings for vendored code
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(glad PRIVATE -w)
endif()

# Find Python for resource compiler
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Generate internal resources for library
set(ZUI_INTERNAL_RES_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/zui_internal")
set(ZUI_INTERNAL_RES_H "${ZUI_INTERNAL_RES_DIR}/zui_internal_resources.h")
set(ZUI_INTERNAL_RES_C "${ZUI_INTERNAL_RES_DIR}/zui_internal_resources.c")

add_custom_command(
  OUTPUT ${ZUI_INTERNAL_RES_H} ${ZUI_INTERNAL_RES_C}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${ZUI_INTERNAL_RES_DIR}
  COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/rescompile.py
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal_resources.res
    -o zui_internal_resources
    -d ${ZUI_INTERNAL_RES_DIR}
    -p zui_internal
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal_resources.res
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/rescompile.py
  COMMENT "Compiling ZUI internal resources"
  VERBATIM
)

add_custom_target(zui_internal_resources DEPENDS ${ZUI_INTERNAL_RES_H} ${ZUI_INTERNAL_RES_C})

# ZUI library
add_library(zui STATIC
  # Core
  src/core/app.c
  src/core/window.c
  src/core/window_decoration.c
  src/core/widget.c
  src/core/widgets.c
  src/core/layout.c
  src/core/icon.c
  src/core/font.c
  src/core/image.c
  src/core/panel.c
  src/core/path.c
  src/core/shape.c
  src/core/resource.c

  # Platform - Wayland
  src/platform/wayland/wayland_platform.c
  src/platform/wayland/protocols/xdg-shell-protocol.c
  src/platform/wayland/protocols/xdg-decoration-unstable-v1-protocol.c

  # Renderer - OpenGL
  src/renderer/opengl/egl_context.c
  src/renderer/opengl/renderer.c

  # Video
  src/core/video.c
  src/video/video_backend.c

  # Audio
  src/audio/audio.c

  # Generated internal resources
  ${ZUI_INTERNAL_RES_C}
)

add_dependencies(zui zui_internal_resources)

# Video backends
if(ZUI_VIDEO_MPV)
  target_sources(zui PRIVATE src/video/backends/mpv_backend.c)
  target_compile_definitions(zui PRIVATE ZUI_HAS_MPV)
  target_link_libraries(zui PRIVATE ${MPV_LIBRARIES})
  target_include_directories(zui PRIVATE ${MPV_INCLUDE_DIRS})
endif()

target_include_directories(zui
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio
    ${ZUI_INTERNAL_RES_DIR}
    ${WAYLAND_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${XKB_INCLUDE_DIRS}
    ${WEBP_INCLUDE_DIRS}
)

target_link_libraries(zui
  PUBLIC
    glad
  PRIVATE
    ${WAYLAND_LIBRARIES}
    ${EGL_LIBRARIES}
    ${XKB_LIBRARIES}
    ${WEBP_LIBRARIES}
    OpenGL::OpenGL
    m
    pthread
    dl
)

target_compile_definitions(zui PRIVATE
  $<$<CONFIG:Debug>:ZUI_DEBUG>
)

# Suppress warnings for vendored headers (nanosvg, stb_truetype)
set_source_files_properties(src/core/icon.c PROPERTIES
  COMPILE_FLAGS "-Wno-sign-conversion -Wno-conversion -Wno-double-promotion -Wno-shadow -Wno-sign-compare"
)
set_source_files_properties(src/core/font.c PROPERTIES
  COMPILE_FLAGS "-Wno-sign-conversion -Wno-conversion -Wno-double-promotion -Wno-shadow -Wno-sign-compare -Wno-unused-function"
)
set_source_files_properties(src/core/image.c PROPERTIES
  COMPILE_FLAGS "-Wno-sign-conversion -Wno-conversion -Wno-double-promotion -Wno-shadow -Wno-sign-compare -Wno-unused-function"
)
set_source_files_properties(src/audio/audio.c PROPERTIES
  COMPILE_FLAGS "-Wno-sign-conversion -Wno-conversion -Wno-double-promotion -Wno-shadow -Wno-sign-compare -Wno-unused-function -Wno-pedantic"
)

# Copy shader files to build directory
file(GLOB SHADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*")
file(COPY ${SHADER_FILES} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders")

# Also copy during build for incremental builds
add_custom_target(copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/assets/icons"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/assets/logo"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/assets/video"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
    "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons"
    "${CMAKE_CURRENT_BINARY_DIR}/assets/icons"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo"
    "${CMAKE_CURRENT_BINARY_DIR}/assets/logo"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/video"
    "${CMAKE_CURRENT_BINARY_DIR}/assets/video"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/assets/audio"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio"
    "${CMAKE_CURRENT_BINARY_DIR}/assets/audio"
  COMMENT "Copying asset files"
)

# Resource compiler function for applications
function(zui_compile_resources TARGET RES_FILE)
  get_filename_component(RES_NAME ${RES_FILE} NAME_WE)
  get_filename_component(RES_DIR ${RES_FILE} DIRECTORY)

  set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/${TARGET}")
  set(GEN_HEADER "${GEN_DIR}/app_resources.h")
  set(GEN_SOURCE "${GEN_DIR}/app_resources.c")

  add_custom_command(
    OUTPUT ${GEN_HEADER} ${GEN_SOURCE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}
    COMMAND ${Python3_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/tools/rescompile.py
      ${RES_FILE}
      -o app_resources
      -d ${GEN_DIR}
      -p zui_app
    DEPENDS ${RES_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/rescompile.py
    COMMENT "Compiling resources: ${RES_FILE}"
    VERBATIM
  )

  target_sources(${TARGET} PRIVATE ${GEN_SOURCE})
  target_include_directories(${TARGET} PRIVATE ${GEN_DIR})
endfunction()

# Examples
option(ZUI_BUILD_EXAMPLES "Build example applications" ON)

if(ZUI_BUILD_EXAMPLES)
  add_executable(zui_example examples/main.c)
  target_link_libraries(zui_example PRIVATE zui)
  add_dependencies(zui_example copy_assets)

  # Compile embedded resources
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/resources.res")
    zui_compile_resources(zui_example "${CMAKE_CURRENT_SOURCE_DIR}/examples/resources.res")
  endif()

endif()

# =============================================================================
# Installation
# =============================================================================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install library and dependencies
install(TARGETS zui glad
  EXPORT zuiTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install public headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/zui
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
  PATTERN "internal" EXCLUDE
)

# Install generated version header
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/zui/version.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/zui
)

# Install glad headers (vendored dependency)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

# Install assets
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/zui
  PATTERN "*.ttf"
  PATTERN "*.svg"
  PATTERN "*.png"
  PATTERN "*.vert"
  PATTERN "*.frag"
)

# Export targets
install(EXPORT zuiTargets
  FILE zuiTargets.cmake
  NAMESPACE zui::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zui
)

# Generate config file
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/zuiConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/zuiConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zui
)

# Generate version file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/zuiConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Install CMake config files
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/zuiConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/zuiConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zui
)

# Generate and install pkg-config file
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/zui.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/zui.pc
  @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zui.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Summary
message(STATUS "")
message(STATUS "ZUI ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Wayland:    ${WAYLAND_VERSION}")
message(STATUS "  EGL:        ${EGL_VERSION}")
message(STATUS "  Examples:   ${ZUI_BUILD_EXAMPLES}")
message(STATUS "  Video MPV:  ${ZUI_VIDEO_MPV}")
message(STATUS "")
