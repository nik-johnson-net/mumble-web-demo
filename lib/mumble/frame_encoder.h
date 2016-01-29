#ifndef _MUMBLE_FRAME_ENCODER_H_
#define _MUMBLE_FRAME_ENCODER_H_

#include "proto/Mumble.pb-c.h"

typedef struct {
  char *buffer;
  unsigned buffer_allocated;
  unsigned buffer_size;
} mumble_frame_encoder_t;

int mumble_frame_encoder_init(mumble_frame_encoder_t *ctx);
void mumble_frame_encoder_free(mumble_frame_encoder_t *ctx);
void mumble_frame_encode(mumble_frame_encoder_t *ctx, ProtobufCMessage *message);

#endif
