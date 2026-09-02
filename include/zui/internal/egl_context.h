#ifndef ZUI_EGL_CONTEXT_H
#define ZUI_EGL_CONTEXT_H

#include <glad/egl.h>
#include <glad/glad.h>
#include <stdbool.h>
#include <wayland-egl.h>

typedef struct ZuiEglContext {
    EGLDisplay display;
    EGLContext context;
    EGLConfig config;
} ZuiEglContext;

typedef struct ZuiEglSurface {
    EGLSurface surface;
} ZuiEglSurface;

bool zui_egl_init(ZuiEglContext *ctx, struct wl_display *wl_display);
void zui_egl_shutdown(ZuiEglContext *ctx);

bool zui_egl_surface_create(ZuiEglContext *ctx, ZuiEglSurface *surface,
                             struct wl_egl_window *egl_window);
void zui_egl_surface_destroy(ZuiEglContext *ctx, ZuiEglSurface *surface);

void zui_egl_make_current(ZuiEglContext *ctx, ZuiEglSurface *surface);
void zui_egl_swap_buffers(ZuiEglContext *ctx, ZuiEglSurface *surface);

#endif
