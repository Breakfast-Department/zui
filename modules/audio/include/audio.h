#ifndef ZUI_AUDIO_H
#define ZUI_AUDIO_H

#include <stdbool.h>

typedef struct ZuiAudio ZuiAudio;

bool zui_audio_init(void);
void zui_audio_shutdown(void);

ZuiAudio *zui_audio_load(const char *path);
void zui_audio_destroy(ZuiAudio *audio);

void zui_audio_play(ZuiAudio *audio);
void zui_audio_stop(ZuiAudio *audio);
void zui_audio_set_volume(ZuiAudio *audio, float volume);
void zui_audio_set_loop(ZuiAudio *audio, bool loop);
bool zui_audio_is_playing(ZuiAudio *audio);

#endif
