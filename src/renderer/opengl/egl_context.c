#include <zui/internal/egl_context.h>
#include <stdio.h>
#include <string.h>

static void *egl_get_proc(const char *name)
{
    union {
        void (*fn)(void);
        void *ptr;
    } result;
    result.fn = eglGetProcAddress(name);
    return result.ptr;
}

bool zui_egl_init(ZuiEglContext *ctx, struct wl_display *wl_display)
{
    memset(ctx, 0, sizeof(*ctx));

    if (!gladLoadEGLLoader(egl_get_proc)) {
        fprintf(stderr, "ZUI: Failed to load EGL\n");
        return false;
    }

    ctx->display = eglGetDisplay((EGLNativeDisplayType)wl_display);
    if (ctx->display == EGL_NO_DISPLAY) {
        fprintf(stderr, "ZUI: Failed to get EGL display\n");
        return false;
    }

    EGLint major, minor;
    if (!eglInitialize(ctx->display, &major, &minor)) {
        fprintf(stderr, "ZUI: Failed to initialize EGL\n");
        return false;
    }

    if (!eglBindAPI(EGL_OPENGL_API)) {
        fprintf(stderr, "ZUI: Failed to bind OpenGL API\n");
        return false;
    }

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint num_configs;
    if (!eglChooseConfig(ctx->display, config_attribs, &ctx->config, 1,
                          &num_configs) || num_configs == 0) {
        fprintf(stderr, "ZUI: Failed to choose EGL config\n");
        return false;
    }

    EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    ctx->context = eglCreateContext(ctx->display, ctx->config, EGL_NO_CONTEXT,
                                     context_attribs);
    if (ctx->context == EGL_NO_CONTEXT) {
        fprintf(stderr, "ZUI: Failed to create EGL context\n");
        return false;
    }

    return true;
}

void zui_egl_shutdown(ZuiEglContext *ctx)
{
    if (ctx->context != EGL_NO_CONTEXT) {
        eglDestroyContext(ctx->display, ctx->context);
    }
    if (ctx->display != EGL_NO_DISPLAY) {
        eglTerminate(ctx->display);
    }
}

bool zui_egl_surface_create(ZuiEglContext *ctx, ZuiEglSurface *surface,
                             struct wl_egl_window *egl_window)
{
    surface->surface = eglCreateWindowSurface(ctx->display, ctx->config,
                                               (EGLNativeWindowType)egl_window,
                                               NULL);
    if (surface->surface == EGL_NO_SURFACE) {
        fprintf(stderr, "ZUI: Failed to create EGL surface\n");
        return false;
    }
    return true;
}

void zui_egl_surface_destroy(ZuiEglContext *ctx, ZuiEglSurface *surface)
{
    if (surface->surface != EGL_NO_SURFACE) {
        eglDestroySurface(ctx->display, surface->surface);
        surface->surface = EGL_NO_SURFACE;
    }
}

void zui_egl_make_current(ZuiEglContext *ctx, ZuiEglSurface *surface)
{
    eglMakeCurrent(ctx->display, surface->surface, surface->surface,
                    ctx->context);
}

void zui_egl_swap_buffers(ZuiEglContext *ctx, ZuiEglSurface *surface)
{
    eglSwapBuffers(ctx->display, surface->surface);
}
