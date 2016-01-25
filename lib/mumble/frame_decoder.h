#ifndef _MUMBLE_FRAME_DECODER_H_
#define _MUMBLE_FRAME_DECODER_H_

#include "proto/Mumble.pb-c.h"

typedef struct {
  char *buffer;
  unsigned buffer_allocated;
  unsigned buffer_size;
} mumble_frame_decoder_t;

int mumble_frame_init(mumble_frame_decoder_t *ctx);
void mumble_frame_free(mumble_frame_decoder_t *ctx);
int mumble_frame_size(mumble_frame_decoder_t *ctx);
int mumble_frame_type(mumble_frame_decoder_t *ctx);
int mumble_frame_ready(mumble_frame_decoder_t *ctx);
int mumble_frame_is_audio(mumble_frame_decoder_t *ctx);
int mumble_frame_append(mumble_frame_decoder_t *ctx, const char* buf, unsigned size);
int mumble_frame_decode(mumble_frame_decoder_t *ctx, ProtobufCMessage **message);
int mumble_frame_pop(mumble_frame_decoder_t *ctx);

#endif
