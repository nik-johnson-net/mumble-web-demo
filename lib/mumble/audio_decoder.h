#ifndef _MUMBLE_AUDIO_DECODER_H_
#define _MUMBLE_AUDIO_DECODER_H_

#include "codec/opus.h"
#include "common.h"

#include <stdint.h>

struct _mumble_audio_decoder_t;

typedef void (*mumble_decoder_cb)(struct _mumble_audio_decoder_t *decoder, void* data, const audio_packet_t *audio);

struct _mumble_decoder_cb_t {
  mumble_decoder_cb cb;
  void *data;
};

struct _mumble_audio_decoder_t {
  struct _mumble_decoder_cb_t cb;
  codec_opus_decoder_t *opus;
  unsigned int sequence;
  unsigned int packets;
  unsigned int dropped_sequence;
};

typedef struct _mumble_audio_decoder_t mumble_audio_decoder_t;

/* Init the object */
void mumble_audio_decoder_init(mumble_audio_decoder_t *decoder);

/* Decode a mumble audio packet */
int mumble_audio_decoder_decode(mumble_audio_decoder_t *decoder, const char *buf, int len, audio_packet_t *audio);

/* Set the function to be run on new packets */
void mumble_audio_decoder_set_cb(mumble_audio_decoder_t *decoder, mumble_decoder_cb cb, void *data);

#endif
