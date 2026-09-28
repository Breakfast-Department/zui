#ifndef ZUI_VIDEO_INTERNAL_H
#define ZUI_VIDEO_INTERNAL_H

#include "widget_internal.h"
#include "video_backend.h"
#include "renderer.h"
#include <zui/video.h>

struct ZuiVideo {
  ZuiWidget base;

  ZuiVideoBackend *backend;
  ZuiTexture texture;

  bool preserve_aspect;
  bool loop;

  int video_width;
  int video_height;
  double duration;

  ZuiVideoCallback on_end;
  void *on_end_data;
  ZuiVideoCallback on_error;
  void *on_error_data;

  ZuiVideoState last_state;
};

void zui_video_update_frame(ZuiVideo *video);

#endif
