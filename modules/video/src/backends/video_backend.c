#include <zui/internal/video_backend.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BACKENDS 8

typedef struct {
  const char *name;
  ZuiVideoBackendCreateFunc create;
} BackendEntry;

static BackendEntry g_backends[MAX_BACKENDS];
static int g_backend_count = 0;
static const char *g_default_backend = NULL;

void zui_video_backend_register(const char *name, ZuiVideoBackendCreateFunc create)
{
  if (g_backend_count >= MAX_BACKENDS) return;

  g_backends[g_backend_count].name = name;
  g_backends[g_backend_count].create = create;
  g_backend_count++;

  if (!g_default_backend) {
    g_default_backend = name;
  }
}

ZuiVideoBackend *zui_video_backend_create(const char *name)
{
  if (!name) return NULL;

  for (int i = 0; i < g_backend_count; i++) {
    if (strcmp(g_backends[i].name, name) == 0) {
      return g_backends[i].create();
    }
  }

  return NULL;
}

ZuiVideoBackend *zui_video_backend_create_default(void)
{
  if (!g_default_backend) return NULL;
  return zui_video_backend_create(g_default_backend);
}

#ifdef ZUI_HAS_MPV
extern void zui_mpv_backend_register(void);
#endif

void zui_video_backends_init(void)
{
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

#ifdef ZUI_HAS_MPV
  zui_mpv_backend_register();
#endif
}
