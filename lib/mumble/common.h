#ifndef _MUMBLE_AUDIO_COMMON_H_
#define _MUMBLE_AUDIO_COMMON_H_

#define MUMBLE_AUDIO_TYPE(x) ((x & 0xE0) >> 5)
#define MUMBLE_AUDIO_CELT_ALPHA 0
#define MUMBLE_AUDIO_PING 1
#define MUMBLE_AUDIO_SPEEX 2
#define MUMBLE_AUDIO_CELT_BETA 3
#define MUMBLE_AUDIO_OPUS 4

#define MUMBLE_AUDIO_TARGET(x) (x & 0x1F)
#define MUMBLE_AUDIO_NORMAL 0
#define MUMBLE_AUDIO_LOOPBACK 31
#define MUMBLE_AUDIO_CHANNEL_WHISPER 1
#define MUMBLE_AUDIO_USER_WHISPER 2

#include <stdint.h>

/* For positional audio */
typedef struct {
  float x;
  float y;
  float z;
} position_t;

/* For pcm data streams */
typedef struct {
  uint16_t *data;
  int hz;
  int samples;
} pcm_t;

#endif
