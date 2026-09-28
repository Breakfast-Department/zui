#include <zui/internal/video_backend.h>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <glad/glad.h>
#include <EGL/egl.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  mpv_handle *mpv;
  mpv_render_context *render_ctx;
  ZuiVideoState state;
  double position;
  double duration;
  int width;
  int height;
  double fps;
  bool file_loaded;
  bool play_pending;
} MpvBackendImpl;

static void *get_proc_address(void *ctx, const char *name)
{
  (void)ctx;
  return (void *)eglGetProcAddress(name);
}

static void process_mpv_events(ZuiVideoBackend *backend)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->mpv) return;

  while (1) {
    mpv_event *event = mpv_wait_event(impl->mpv, 0);
    if (event->event_id == MPV_EVENT_NONE) break;

    switch (event->event_id) {
      case MPV_EVENT_FILE_LOADED:
        impl->file_loaded = true;
        impl->state = ZUI_VIDEO_PAUSED;

        mpv_get_property(impl->mpv, "duration", MPV_FORMAT_DOUBLE, &impl->duration);
        mpv_get_property(impl->mpv, "width", MPV_FORMAT_INT64, &impl->width);
        mpv_get_property(impl->mpv, "height", MPV_FORMAT_INT64, &impl->height);
        mpv_get_property(impl->mpv, "container-fps", MPV_FORMAT_DOUBLE, &impl->fps);

        if (impl->play_pending) {
          impl->play_pending = false;
          int pause = 0;
          mpv_set_property(impl->mpv, "pause", MPV_FORMAT_FLAG, &pause);
          impl->state = ZUI_VIDEO_PLAYING;
        }
        break;

      case MPV_EVENT_END_FILE: {
        mpv_event_end_file *end = event->data;
        if (end->reason == MPV_END_FILE_REASON_EOF) {
          impl->state = ZUI_VIDEO_ENDED;
        } else if (end->reason == MPV_END_FILE_REASON_ERROR) {
          impl->state = ZUI_VIDEO_ERROR;
        }
        break;
      }

      case MPV_EVENT_PLAYBACK_RESTART:
        if (impl->state == ZUI_VIDEO_PAUSED || impl->state == ZUI_VIDEO_STOPPED) {
          impl->state = ZUI_VIDEO_PAUSED;
        }
        break;

      case MPV_EVENT_SHUTDOWN:
        impl->state = ZUI_VIDEO_STOPPED;
        break;

      default:
        break;
    }
  }

  if (impl->file_loaded) {
    mpv_get_property(impl->mpv, "time-pos", MPV_FORMAT_DOUBLE, &impl->position);

    int eof_reached = 0;
    if (mpv_get_property(impl->mpv, "eof-reached", MPV_FORMAT_FLAG, &eof_reached) >= 0) {
      if (eof_reached && impl->state == ZUI_VIDEO_PLAYING) {
        impl->state = ZUI_VIDEO_ENDED;
      }
    }
  }
}

static bool mpv_backend_open(ZuiVideoBackend *backend, const char *path)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->mpv) return false;

  impl->file_loaded = false;
  impl->state = ZUI_VIDEO_STOPPED;
  impl->position = 0;
  impl->duration = 0;

  const char *cmd[] = {"loadfile", path, NULL};
  return mpv_command(impl->mpv, cmd) >= 0;
}

static void mpv_backend_close(ZuiVideoBackend *backend)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->mpv) return;

  const char *cmd[] = {"stop", NULL};
  mpv_command(impl->mpv, cmd);
  impl->state = ZUI_VIDEO_STOPPED;
  impl->file_loaded = false;
}

static void mpv_backend_destroy(ZuiVideoBackend *backend)
{
  MpvBackendImpl *impl = backend->impl;

  if (impl->render_ctx) {
    mpv_render_context_free(impl->render_ctx);
  }

  if (impl->mpv) {
    mpv_terminate_destroy(impl->mpv);
  }

  free(impl);
  free(backend);
}

static bool mpv_backend_play(ZuiVideoBackend *backend)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->mpv) return false;

  if (!impl->file_loaded) {
    impl->play_pending = true;
    return true;
  }

  int pause = 0;
  mpv_set_property(impl->mpv, "pause", MPV_FORMAT_FLAG, &pause);
  impl->state = ZUI_VIDEO_PLAYING;
  return true;
}

static bool mpv_backend_pause(ZuiVideoBackend *backend)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->mpv || !impl->file_loaded) return false;

  int pause = 1;
  mpv_set_property(impl->mpv, "pause", MPV_FORMAT_FLAG, &pause);
  impl->state = ZUI_VIDEO_PAUSED;
  return true;
}

static bool mpv_backend_stop(ZuiVideoBackend *backend)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->mpv) return false;

  const char *cmd[] = {"stop", NULL};
  mpv_command(impl->mpv, cmd);
  impl->state = ZUI_VIDEO_STOPPED;
  return true;
}

static bool mpv_backend_seek(ZuiVideoBackend *backend, double position)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->mpv || !impl->file_loaded) return false;

  char pos_str[64];
  snprintf(pos_str, sizeof(pos_str), "%f", position);
  const char *cmd[] = {"seek", pos_str, "absolute", NULL};
  int result = mpv_command(impl->mpv, cmd);

  if (result >= 0 && impl->state == ZUI_VIDEO_ENDED) {
    impl->state = ZUI_VIDEO_PAUSED;
  }

  return result >= 0;
}

static bool mpv_backend_render_to_texture(ZuiVideoBackend *backend, GLuint texture,
                                          int width, int height)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->render_ctx) return false;

  GLint prev_fbo, prev_vao, prev_program;
  GLint prev_viewport[4];
  GLboolean prev_blend, prev_scissor;

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prev_vao);
  glGetIntegerv(GL_CURRENT_PROGRAM, &prev_program);
  glGetIntegerv(GL_VIEWPORT, prev_viewport);
  glGetBooleanv(GL_BLEND, &prev_blend);
  glGetBooleanv(GL_SCISSOR_TEST, &prev_scissor);

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, texture, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prev_fbo);
    glDeleteFramebuffers(1, &fbo);
    return false;
  }

  mpv_opengl_fbo fbo_params = {
    .fbo = (int)fbo,
    .w = width,
    .h = height,
  };

  int flip_y = 0;
  mpv_render_param params[] = {
    {MPV_RENDER_PARAM_OPENGL_FBO, &fbo_params},
    {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
    {0}
  };

  int result = mpv_render_context_render(impl->render_ctx, params);

  glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prev_fbo);
  glDeleteFramebuffers(1, &fbo);

  glBindVertexArray((GLuint)prev_vao);
  glUseProgram((GLuint)prev_program);
  glViewport(prev_viewport[0], prev_viewport[1],
             prev_viewport[2], prev_viewport[3]);

  if (prev_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
  if (prev_scissor) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);

  return result >= 0;
}

static ZuiVideoState mpv_backend_get_state(ZuiVideoBackend *backend)
{
  MpvBackendImpl *impl = backend->impl;
  return impl->state;
}

static double mpv_backend_get_position(ZuiVideoBackend *backend)
{
  MpvBackendImpl *impl = backend->impl;
  return impl->position;
}

static bool mpv_backend_get_info(ZuiVideoBackend *backend, ZuiVideoInfo *info)
{
  MpvBackendImpl *impl = backend->impl;
  if (!impl->file_loaded) return false;

  info->width = impl->width;
  info->height = impl->height;
  info->duration = impl->duration;
  info->fps = impl->fps;
  return true;
}

static void mpv_backend_update(ZuiVideoBackend *backend)
{
  process_mpv_events(backend);
}

static const ZuiVideoBackendVTable mpv_vtable = {
  .open = mpv_backend_open,
  .close = mpv_backend_close,
  .destroy = mpv_backend_destroy,
  .play = mpv_backend_play,
  .pause = mpv_backend_pause,
  .stop = mpv_backend_stop,
  .seek = mpv_backend_seek,
  .render_to_texture = mpv_backend_render_to_texture,
  .get_state = mpv_backend_get_state,
  .get_position = mpv_backend_get_position,
  .get_info = mpv_backend_get_info,
  .update = mpv_backend_update,
};

static ZuiVideoBackend *mpv_backend_create(void)
{
  ZuiVideoBackend *backend = calloc(1, sizeof(ZuiVideoBackend));
  if (!backend) return NULL;

  MpvBackendImpl *impl = calloc(1, sizeof(MpvBackendImpl));
  if (!impl) {
    free(backend);
    return NULL;
  }

  impl->mpv = mpv_create();
  if (!impl->mpv) {
    free(impl);
    free(backend);
    return NULL;
  }

  mpv_set_option_string(impl->mpv, "vo", "libmpv");
  mpv_set_option_string(impl->mpv, "ao", "null");
  mpv_set_option_string(impl->mpv, "audio", "no");
  mpv_set_option_string(impl->mpv, "pause", "yes");
  mpv_set_option_string(impl->mpv, "keep-open", "yes");

  if (mpv_initialize(impl->mpv) < 0) {
    mpv_destroy(impl->mpv);
    free(impl);
    free(backend);
    return NULL;
  }

  mpv_opengl_init_params gl_init_params = {
    .get_proc_address = get_proc_address,
  };

  mpv_render_param params[] = {
    {MPV_RENDER_PARAM_API_TYPE, MPV_RENDER_API_TYPE_OPENGL},
    {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
    {0}
  };

  if (mpv_render_context_create(&impl->render_ctx, impl->mpv, params) >= 0) {
    backend->supports_gl_render = true;
  }

  impl->state = ZUI_VIDEO_STOPPED;
  backend->impl = impl;
  backend->vtable = &mpv_vtable;

  return backend;
}

void zui_mpv_backend_register(void)
{
  zui_video_backend_register("mpv", mpv_backend_create);
}
