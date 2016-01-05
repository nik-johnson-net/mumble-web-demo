#ifndef _MUMBLE_AUDIO_ENCODER_H_
#define _MUMBLE_AUDIO_ENCODER_H_

#include "common.h"

struct _mumble_audio_encoder_t;

typedef struct {
  int target;
  char *audio;
  int audio_len;
  position_t position;
} audio_encoded_t;

typedef void (*mumble_encoder_cb)(struct _mumble_audio_encoder_t *encoder, void* data, const audio_encoded_t *audio);

struct _mumble_encoder_cb_t {
  mumble_encoder_cb cb;
  void *data;
};

struct _mumble_audio_encoder_t {
  struct _mumble_encoder_cb_t cb;
};

typedef struct _mumble_audio_encoder_t mumble_audio_encoder_t;

/* Init the object */
void mumble_audio_encoder_init(mumble_audio_encoder_t *encoder);

/* encode a mumble audio packet */
audio_encoded_t mumble_audio_encoder_encode(mumble_audio_encoder_t *encoder, const pcm_t *pcm);

/* Set the function to be run on new packets */
void mumble_audio_encoder_set_cb(mumble_audio_encoder_t *encoder, mumble_encoder_cb cb, void *data);

#endif
