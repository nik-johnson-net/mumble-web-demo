#ifndef _MUMBLE_AUDIO_ENCODER_H_
#define _MUMBLE_AUDIO_ENCODER_H_

#include "common.h"
#include "codec/opus.h"

struct _mumble_audio_encoder_t;

typedef void (*mumble_encoder_cb)(struct _mumble_audio_encoder_t *encoder, void* data, const char *audio, unsigned size);

struct _mumble_encoder_cb_t {
  mumble_encoder_cb cb;
  void *data;
};

struct _mumble_audio_encoder_t {
  struct _mumble_encoder_cb_t cb;
  codec_opus_encoder_t *opus;
  unsigned int sequence;
};

typedef struct _mumble_audio_encoder_t mumble_audio_encoder_t;

/* Init the object */
void mumble_audio_encoder_init(mumble_audio_encoder_t *encoder, unsigned short sample_rate, unsigned short buffer_ms, int app);

/* encode a mumble audio packet */
void mumble_audio_encoder_encode(mumble_audio_encoder_t *encoder, unsigned target, const pcm_t *pcm, const position_t *position);

/* Set the function to be run on new packets */
void mumble_audio_encoder_set_cb(mumble_audio_encoder_t *encoder, mumble_encoder_cb cb, void *data);

#endif
