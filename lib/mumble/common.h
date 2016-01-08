#ifndef _MUMBLE_AUDIO_COMMON_H_
#define _MUMBLE_AUDIO_COMMON_H_

#define MUMBLE_TYPE_VERSION 0
#define MUMBLE_TYPE_UDPTUNNEL 1
#define MUMBLE_TYPE_AUTHENTICATE 2
#define MUMBLE_TYPE_PING 3
#define MUMBLE_TYPE_REJECT 4
#define MUMBLE_TYPE_SERVER_SYNC 5
#define MUMBLE_TYPE_CHANNEL_REMOVE 6
#define MUMBLE_TYPE_CHANNEL_STATE 7
#define MUMBLE_TYPE_USER_REMOVE 8
#define MUMBLE_TYPE_USER_STATE 9
#define MUMBLE_TYPE_BAN_LIST 10
#define MUMBLE_TYPE_TEXT_MESSAGE 11
#define MUMBLE_TYPE_PERMISSION_DENIED 12
#define MUMBLE_TYPE_ACL 13
#define MUMBLE_TYPE_QUERY_USERS 14
#define MUMBLE_TYPE_CRYPT_SETUP 15
#define MUMBLE_TYPE_CONTEXT_ACTION_MODIFY 16
#define MUMBLE_TYPE_CONTEXT_ACTION 17
#define MUMBLE_TYPE_USER_LIST 18
#define MUMBLE_TYPE_VOICE_TARGET 19
#define MUMBLE_TYPE_PERMISSION_QUERY 20
#define MUMBLE_TYPE_CODEC_VERSION 21
#define MUMBLE_TYPE_USER_STATS 22
#define MUMBLE_TYPE_REQUEST_BLOB 23
#define MUMBLE_TYPE_SERVER_CONFIG 24
#define MUMBLE_TYPE_SUGGEST_CONFIG 25
#define MUMBLE_TYPE_COUNT 26 /* ot an actual type */


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

typedef struct {
  uint8_t codec;
  pcm_t pcm;
  position_t position;
  uint64_t sequence;
  uint64_t source;
  uint64_t target;
} audio_packet_t;

#endif
