#include <zui/internal/video_internal.h>
#include <stdlib.h>
#include <stdio.h>

static void video_draw(ZuiWidget *widget, ZuiRenderer *renderer)
{
  ZuiVideo *video = (ZuiVideo *)widget;

  if (!video->backend) return;

  zui_video_update_frame(video);

  if (!video->texture.id) return;

  ZuiRect draw_rect;
  if (video->preserve_aspect && video->video_width > 0 && video->video_height > 0) {
    float widget_aspect = widget->bounds.width / widget->bounds.height;
    float video_aspect = (float)video->video_width / (float)video->video_height;

    if (video_aspect > widget_aspect) {
      draw_rect.w = widget->bounds.width;
      draw_rect.h = draw_rect.w / video_aspect;
      draw_rect.x = widget->bounds.x;
      draw_rect.y = widget->bounds.y + (widget->bounds.height - draw_rect.h) / 2;
    } else {
      draw_rect.h = widget->bounds.height;
      draw_rect.w = draw_rect.h * video_aspect;
      draw_rect.x = widget->bounds.x + (widget->bounds.width - draw_rect.w) / 2;
      draw_rect.y = widget->bounds.y;
    }
  } else {
    draw_rect = ZUI_RECT(widget->bounds.x, widget->bounds.y,
                         widget->bounds.width, widget->bounds.height);
  }

  ZuiColor white = {1.0f, 1.0f, 1.0f, 1.0f};
  zui_renderer_draw_texture(renderer, &video->texture, draw_rect, white);
}

static void video_destroy(ZuiWidget *widget)
{
  ZuiVideo *video = (ZuiVideo *)widget;

  if (video->texture.id) {
    zui_texture_destroy(&video->texture);
  }

  if (video->backend) {
    video->backend->vtable->close(video->backend);
    video->backend->vtable->destroy(video->backend);
  }
}

static const ZuiWidgetVTable video_vtable = {
  .draw = video_draw,
  .destroy = video_destroy,
};

void zui_video_update_frame(ZuiVideo *video)
{
  if (!video->backend) return;

  video->backend->vtable->update(video->backend);

  ZuiVideoState state = video->backend->vtable->get_state(video->backend);

  if (state == ZUI_VIDEO_ENDED && video->last_state != ZUI_VIDEO_ENDED) {
    if (video->loop) {
      video->backend->vtable->seek(video->backend, 0);
      video->backend->vtable->play(video->backend);
    } else if (video->on_end) {
      video->on_end(video, video->on_end_data);
    }
  }

  if (state == ZUI_VIDEO_ERROR && video->last_state != ZUI_VIDEO_ERROR) {
    if (video->on_error) {
      video->on_error(video, video->on_error_data);
    }
  }

  video->last_state = state;

  if (state != ZUI_VIDEO_PLAYING && state != ZUI_VIDEO_PAUSED) return;

  ZuiVideoInfo info;
  if (video->backend->vtable->get_info(video->backend, &info)) {
    if (info.width != video->video_width || info.height != video->video_height) {
      video->video_width = info.width;
      video->video_height = info.height;
      video->duration = info.duration;

      if (video->texture.id) {
        zui_texture_destroy(&video->texture);
      }
      video->texture = zui_texture_create_empty(info.width, info.height);
    }
  }

  if (video->backend->supports_gl_render && video->texture.id) {
    video->backend->vtable->render_to_texture(video->backend, video->texture.id,
                                              video->video_width, video->video_height);
  }
}

ZuiVideo *zui_video_create(const char *backend_name)
{
  zui_video_backends_init();

  ZuiVideo *video = (ZuiVideo *)zui_widget_create(
    sizeof(ZuiVideo), ZUI_WIDGET_VIDEO, &video_vtable);

  if (!video) {
    fprintf(stderr, "ZUI Video: failed to create widget\n");
    return NULL;
  }

  video->preserve_aspect = true;
  video->loop = false;
  video->last_state = ZUI_VIDEO_STOPPED;

  if (backend_name) {
    video->backend = zui_video_backend_create(backend_name);
  } else {
    video->backend = zui_video_backend_create_default();
  }

  if (!video->backend) {
    free(video);
    return NULL;
  }

  return video;
}

void zui_video_destroy(ZuiVideo *video)
{
  if (!video) return;
  zui_widget_destroy((ZuiWidget *)video);
}

bool zui_video_open(ZuiVideo *video, const char *path)
{
  if (!video || !video->backend || !path) return false;
  return video->backend->vtable->open(video->backend, path);
}

void zui_video_close(ZuiVideo *video)
{
  if (!video || !video->backend) return;
  video->backend->vtable->close(video->backend);

  if (video->texture.id) {
    zui_texture_destroy(&video->texture);
    video->texture.id = 0;
  }

  video->video_width = 0;
  video->video_height = 0;
  video->duration = 0;
}

void zui_video_play(ZuiVideo *video)
{
  if (!video || !video->backend) return;

  ZuiVideoState state = video->backend->vtable->get_state(video->backend);

  if (state == ZUI_VIDEO_ENDED || state == ZUI_VIDEO_STOPPED) {
    video->backend->vtable->seek(video->backend, 0);
  }

  video->backend->vtable->play(video->backend);
}

void zui_video_pause(ZuiVideo *video)
{
  if (!video || !video->backend) return;
  video->backend->vtable->pause(video->backend);
}

void zui_video_stop(ZuiVideo *video)
{
  if (!video || !video->backend) return;
  video->backend->vtable->stop(video->backend);
}

void zui_video_toggle_playback(ZuiVideo *video)
{
  if (!video || !video->backend) return;

  ZuiVideoState state = video->backend->vtable->get_state(video->backend);

  if (state == ZUI_VIDEO_PLAYING) {
    zui_video_pause(video);
  } else {
    zui_video_play(video);
  }
}

void zui_video_seek(ZuiVideo *video, double position)
{
  if (!video || !video->backend) return;
  video->backend->vtable->seek(video->backend, position);
}

bool zui_video_is_playing(ZuiVideo *video)
{
  if (!video || !video->backend) return false;
  return video->backend->vtable->get_state(video->backend) == ZUI_VIDEO_PLAYING;
}

bool zui_video_is_paused(ZuiVideo *video)
{
  if (!video || !video->backend) return false;
  return video->backend->vtable->get_state(video->backend) == ZUI_VIDEO_PAUSED;
}

double zui_video_get_position(ZuiVideo *video)
{
  if (!video || !video->backend) return 0;
  return video->backend->vtable->get_position(video->backend);
}

double zui_video_get_duration(ZuiVideo *video)
{
  if (!video) return 0;
  return video->duration;
}

void zui_video_get_dimensions(ZuiVideo *video, int *width, int *height)
{
  if (!video) {
    if (width) *width = 0;
    if (height) *height = 0;
    return;
  }
  if (width) *width = video->video_width;
  if (height) *height = video->video_height;
}

void zui_video_set_size(ZuiVideo *video, float width, float height)
{
  if (!video) return;
  video->base.preferred_size.width = width;
  video->base.preferred_size.height = height;
}

void zui_video_set_preserve_aspect(ZuiVideo *video, bool preserve)
{
  if (!video) return;
  video->preserve_aspect = preserve;
}

void zui_video_set_loop(ZuiVideo *video, bool loop)
{
  if (!video) return;
  video->loop = loop;
}

void zui_video_on_end(ZuiVideo *video, ZuiVideoCallback callback, void *user_data)
{
  if (!video) return;
  video->on_end = callback;
  video->on_end_data = user_data;
}

void zui_video_on_error(ZuiVideo *video, ZuiVideoCallback callback, void *user_data)
{
  if (!video) return;
  video->on_error = callback;
  video->on_error_data = user_data;
}

ZuiWidget *zui_video_as_widget(ZuiVideo *video)
{
  return (ZuiWidget *)video;
}
