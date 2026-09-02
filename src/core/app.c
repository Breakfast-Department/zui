#include <zui/internal/types.h>
#include <zui/internal/widget_internal.h>
#include <zui/internal/wayland_platform.h>
#include <zui/internal/egl_context.h>
#include <zui/internal/renderer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

typedef struct ZuiApp {
    ZuiPlatform platform;
    ZuiEglContext egl;
    ZuiRenderer renderer;
    char shader_path[PATH_MAX];
    bool initialized;
} ZuiApp;

static ZuiApp g_app = {0};

ZuiApp *zui_get_app(void)
{
    return &g_app;
}

static bool find_shader_path(char *out, size_t size)
{
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) return false;
    exe_path[len] = '\0';

    char *last_slash = strrchr(exe_path, '/');
    if (last_slash) *last_slash = '\0';

    snprintf(out, size, "%s/../assets/shaders", exe_path);

    if (access(out, F_OK) == 0) return true;

    snprintf(out, size, "%s/assets/shaders", exe_path);
    if (access(out, F_OK) == 0) return true;

    const char *fallbacks[] = {
        "assets/shaders",
        "../assets/shaders",
        "/usr/share/zui/shaders",
        "/usr/local/share/zui/shaders",
    };
    for (size_t i = 0; i < sizeof(fallbacks) / sizeof(fallbacks[0]); i++) {
        if (access(fallbacks[i], F_OK) == 0) {
            strncpy(out, fallbacks[i], size - 1);
            out[size - 1] = '\0';
            return true;
        }
    }

    return false;
}

bool zui_init(void)
{
    if (g_app.initialized) return true;

    if (!find_shader_path(g_app.shader_path, sizeof(g_app.shader_path))) {
        strncpy(g_app.shader_path, "assets/shaders",
                sizeof(g_app.shader_path) - 1);
    }

    if (!zui_platform_init(&g_app.platform)) {
        return false;
    }

    if (!zui_egl_init(&g_app.egl, g_app.platform.display)) {
        zui_platform_shutdown(&g_app.platform);
        return false;
    }

    g_app.initialized = true;
    return true;
}

void zui_shutdown(void)
{
    if (!g_app.initialized) return;

    zui_renderer_shutdown(&g_app.renderer);
    zui_egl_shutdown(&g_app.egl);
    zui_platform_shutdown(&g_app.platform);
    g_app.initialized = false;
}

void zui_poll_events(void)
{
    if (!g_app.initialized) return;
    zui_platform_poll_events(&g_app.platform);
}

ZuiPlatform *zui_get_platform(void)
{
    return &g_app.platform;
}

ZuiEglContext *zui_get_egl(void)
{
    return &g_app.egl;
}

ZuiRenderer *zui_get_renderer(void)
{
    return &g_app.renderer;
}

bool zui_renderer_is_initialized(void)
{
    return g_app.renderer.rect_shader != 0;
}

bool zui_init_renderer_if_needed(void)
{
    if (g_app.renderer.rect_shader != 0) return true;
    return zui_renderer_init(&g_app.renderer, g_app.shader_path);
}
