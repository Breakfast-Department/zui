cmake_minimum_required(VERSION 3.16)

project(zui
    VERSION 0.1.0
    DESCRIPTION "Lightweight modern GUI toolkit for Wayland"
    LANGUAGES C
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
)

# Find EGL and OpenGL
pkg_check_modules(EGL REQUIRED egl)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# GLAD (vendored OpenGL/EGL loader)
add_library(glad STATIC
    external/glad/src/glad.c
    external/glad/src/egl.c
)
target_include_directories(glad PUBLIC
    external/glad/include
)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# Suppress warnings for vendored code
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(glad PRIVATE -w)
endif()

# ZUI library
add_library(zui STATIC
    src/app.c
    src/window.c
    src/widget.c
    src/layout.c
)

target_include_directories(zui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${WAYLAND_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
)

target_link_libraries(zui
    PUBLIC
        glad
    PRIVATE
        ${WAYLAND_LIBRARIES}
        ${EGL_LIBRARIES}
        OpenGL::OpenGL
)

target_compile_definitions(zui PRIVATE
    $<$<CONFIG:Debug>:ZUI_DEBUG>
)

# Examples
option(ZUI_BUILD_EXAMPLES "Build example applications" ON)

if(ZUI_BUILD_EXAMPLES)
    add_executable(zui_example examples/main.c)
    target_link_libraries(zui_example PRIVATE zui)
endif()

# Summary
message(STATUS "")
message(STATUS "ZUI ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Wayland:    ${WAYLAND_VERSION}")
message(STATUS "  EGL:        ${EGL_VERSION}")
message(STATUS "  Examples:   ${ZUI_BUILD_EXAMPLES}")
message(STATUS "")
