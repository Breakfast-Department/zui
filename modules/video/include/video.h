#ifndef ZUI_VIDEO_H
#define ZUI_VIDEO_H

#include <stdbool.h>

typedef struct ZuiVideo ZuiVideo;
typedef struct ZuiWidget ZuiWidget;

typedef void (*ZuiVideoCallback)(ZuiVideo *video, void *user_data);
typedef void (*ZuiVideoPositionCallback)(ZuiVideo *video, double position,
                                         void *user_data);

ZuiVideo *zui_video_create(const char *backend);
void zui_video_destroy(ZuiVideo *video);

bool zui_video_open(ZuiVideo *video, const char *path);
void zui_video_close(ZuiVideo *video);

void zui_video_play(ZuiVideo *video);
void zui_video_pause(ZuiVideo *video);
void zui_video_stop(ZuiVideo *video);
void zui_video_toggle_playback(ZuiVideo *video);
void zui_video_seek(ZuiVideo *video, double position);

bool zui_video_is_playing(ZuiVideo *video);
bool zui_video_is_paused(ZuiVideo *video);
double zui_video_get_position(ZuiVideo *video);
double zui_video_get_duration(ZuiVideo *video);
void zui_video_get_dimensions(ZuiVideo *video, int *width, int *height);

void zui_video_set_size(ZuiVideo *video, float width, float height);
void zui_video_set_preserve_aspect(ZuiVideo *video, bool preserve);
void zui_video_set_loop(ZuiVideo *video, bool loop);

void zui_video_on_end(ZuiVideo *video, ZuiVideoCallback callback, void *user_data);
void zui_video_on_error(ZuiVideo *video, ZuiVideoCallback callback, void *user_data);

ZuiWidget *zui_video_as_widget(ZuiVideo *video);

#endif
