#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <zui/audio.h>
#include <zui/resource.h>
#include <stdlib.h>
#include <string.h>

static ma_engine g_engine;
static bool g_initialized = false;

struct ZuiAudio {
  ma_sound sound;
  ma_decoder decoder;
  bool loaded;
  bool from_memory;
};

bool zui_audio_init(void)
{
  if (g_initialized) return true;

  ma_result result = ma_engine_init(NULL, &g_engine);
  if (result != MA_SUCCESS) {
    return false;
  }

  g_initialized = true;
  return true;
}

void zui_audio_shutdown(void)
{
  if (!g_initialized) return;
  ma_engine_uninit(&g_engine);
  g_initialized = false;
}

ZuiAudio *zui_audio_load(const char *path)
{
  if (!g_initialized || !path) return NULL;

  ZuiAudio *audio = calloc(1, sizeof(ZuiAudio));
  if (!audio) return NULL;

  ma_result result;

  if (zui_is_resource_path(path)) {
    size_t size;
    const unsigned char *data = zui_resource_get(path, &size);
    if (!data || size == 0) {
      free(audio);
      return NULL;
    }

    ma_decoder_config config = ma_decoder_config_init_default();
    result = ma_decoder_init_memory(data, size, &config, &audio->decoder);
    if (result != MA_SUCCESS) {
      free(audio);
      return NULL;
    }

    result = ma_sound_init_from_data_source(&g_engine, &audio->decoder,
                                             0, NULL, &audio->sound);
    if (result != MA_SUCCESS) {
      ma_decoder_uninit(&audio->decoder);
      free(audio);
      return NULL;
    }
    audio->from_memory = true;
  } else {
    result = ma_sound_init_from_file(&g_engine, path, 0, NULL, NULL, &audio->sound);
    if (result != MA_SUCCESS) {
      free(audio);
      return NULL;
    }
  }

  audio->loaded = true;
  return audio;
}

void zui_audio_destroy(ZuiAudio *audio)
{
  if (!audio) return;
  if (audio->loaded) {
    ma_sound_uninit(&audio->sound);
    if (audio->from_memory) {
      ma_decoder_uninit(&audio->decoder);
    }
  }
  free(audio);
}

void zui_audio_play(ZuiAudio *audio)
{
  if (!audio || !audio->loaded) return;
  ma_sound_seek_to_pcm_frame(&audio->sound, 0);
  ma_sound_start(&audio->sound);
}

void zui_audio_stop(ZuiAudio *audio)
{
  if (!audio || !audio->loaded) return;
  ma_sound_stop(&audio->sound);
}

void zui_audio_set_volume(ZuiAudio *audio, float volume)
{
  if (!audio || !audio->loaded) return;
  ma_sound_set_volume(&audio->sound, volume);
}

void zui_audio_set_loop(ZuiAudio *audio, bool loop)
{
  if (!audio || !audio->loaded) return;
  ma_sound_set_looping(&audio->sound, loop);
}

bool zui_audio_is_playing(ZuiAudio *audio)
{
  if (!audio || !audio->loaded) return false;
  return ma_sound_is_playing(&audio->sound);
}
