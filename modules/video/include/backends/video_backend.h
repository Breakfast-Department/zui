#ifndef ZUI_VIDEO_BACKEND_H
#define ZUI_VIDEO_BACKEND_H

#include <stdbool.h>
#include <stdint.h>
#include <glad/glad.h>

typedef struct ZuiVideoBackend ZuiVideoBackend;

typedef enum ZuiVideoState {
  ZUI_VIDEO_STOPPED,
  ZUI_VIDEO_PLAYING,
  ZUI_VIDEO_PAUSED,
  ZUI_VIDEO_ENDED,
  ZUI_VIDEO_ERROR,
} ZuiVideoState;

typedef struct ZuiVideoInfo {
  int width;
  int height;
  double duration;
  double fps;
} ZuiVideoInfo;

typedef struct ZuiVideoBackendVTable {
  bool (*open)(ZuiVideoBackend *backend, const char *path);
  void (*close)(ZuiVideoBackend *backend);
  void (*destroy)(ZuiVideoBackend *backend);

  bool (*play)(ZuiVideoBackend *backend);
  bool (*pause)(ZuiVideoBackend *backend);
  bool (*stop)(ZuiVideoBackend *backend);
  bool (*seek)(ZuiVideoBackend *backend, double position);

  bool (*render_to_texture)(ZuiVideoBackend *backend, GLuint texture,
                            int width, int height);

  ZuiVideoState (*get_state)(ZuiVideoBackend *backend);
  double (*get_position)(ZuiVideoBackend *backend);
  bool (*get_info)(ZuiVideoBackend *backend, ZuiVideoInfo *info);

  void (*update)(ZuiVideoBackend *backend);
} ZuiVideoBackendVTable;

struct ZuiVideoBackend {
  const ZuiVideoBackendVTable *vtable;
  void *impl;
  bool supports_gl_render;
};

typedef ZuiVideoBackend *(*ZuiVideoBackendCreateFunc)(void);

void zui_video_backend_register(const char *name, ZuiVideoBackendCreateFunc create);
ZuiVideoBackend *zui_video_backend_create(const char *name);
ZuiVideoBackend *zui_video_backend_create_default(void);

void zui_video_backends_init(void);

#endif
