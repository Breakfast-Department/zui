#include <zui/internal/types.h>
#include <zui/internal/widget_internal.h>
#include <zui/internal/wayland_platform.h>
#include <zui/internal/egl_context.h>
#include <zui/internal/renderer.h>
#include "zui/version.h"
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
  char base_path[PATH_MAX];
  bool initialized;
} ZuiApp;

static ZuiApp g_app = {0};

ZuiApp *zui_get_app(void)
{
  return &g_app;
}

static bool find_base_path(char *out, size_t size)
{
  char exe_path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
  if (len == -1) return false;
  exe_path[len] = '\0';

  char *last_slash = strrchr(exe_path, '/');
  if (last_slash) *last_slash = '\0';

  char test_path[PATH_MAX];
  snprintf(test_path, sizeof(test_path), "%s/../assets", exe_path);
  if (access(test_path, F_OK) == 0) {
    snprintf(out, size, "%s/..", exe_path);
    return true;
  }

  snprintf(test_path, sizeof(test_path), "%s/assets", exe_path);
  if (access(test_path, F_OK) == 0) {
    strncpy(out, exe_path, size - 1);
    out[size - 1] = '\0';
    return true;
  }

  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) && access("assets", F_OK) == 0) {
    strncpy(out, cwd, size - 1);
    out[size - 1] = '\0';
    return true;
  }

  return false;
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

  if (!find_base_path(g_app.base_path, sizeof(g_app.base_path))) {
    if (getcwd(g_app.base_path, sizeof(g_app.base_path)) == NULL) {
      strncpy(g_app.base_path, ".", sizeof(g_app.base_path) - 1);
    }
  }

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

const char *zui_get_base_path(void)
{
  return g_app.base_path;
}

bool zui_resolve_asset_path(const char *relative_path, char *out, size_t size)
{
  if (!relative_path || !out || size == 0) return false;

  if (relative_path[0] == '/') {
    strncpy(out, relative_path, size - 1);
    out[size - 1] = '\0';
    return access(out, F_OK) == 0;
  }

  snprintf(out, size, "%s/%s", g_app.base_path, relative_path);
  if (access(out, F_OK) == 0) return true;

  strncpy(out, relative_path, size - 1);
  out[size - 1] = '\0';
  return access(out, F_OK) == 0;
}

const char *zui_version(void)
{
  return ZUI_VERSION;
}

void zui_version_info(int *major, int *minor, int *patch)
{
  if (major) *major = ZUI_VERSION_MAJOR;
  if (minor) *minor = ZUI_VERSION_MINOR;
  if (patch) *patch = ZUI_VERSION_PATCH;
}
