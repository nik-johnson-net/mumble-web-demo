#include "frame_decoder.h"
#include "common.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// TODO: Pick a better default, dynamically resize if needed
#define MUMBLE_FRAME_MAX_SIZE (1<<20)
#define MUMBLE_FRAME_HEADER_SIZE 6

static ProtobufCMessageDescriptor *_descriptors = NULL;
static const ProtobufCMessageDescriptor *descriptors(void) {
  if (_descriptors == NULL) {
    ProtobufCMessageDescriptor tdescriptors[] = {
      mumble_proto__version__descriptor,
      mumble_proto__udptunnel__descriptor,
      mumble_proto__authenticate__descriptor,
      mumble_proto__ping__descriptor,
      mumble_proto__reject__descriptor,
      mumble_proto__server_sync__descriptor,
      mumble_proto__channel_remove__descriptor,
      mumble_proto__channel_state__descriptor,
      mumble_proto__user_remove__descriptor,
      mumble_proto__user_state__descriptor,
      mumble_proto__ban_list__descriptor,
      mumble_proto__text_message__descriptor,
      mumble_proto__permission_denied__descriptor,
      mumble_proto__acl__descriptor,
      mumble_proto__query_users__descriptor,
      mumble_proto__crypt_setup__descriptor,
      mumble_proto__context_action_modify__descriptor,
      mumble_proto__context_action__descriptor,
      mumble_proto__user_list__descriptor,
      mumble_proto__voice_target__descriptor,
      mumble_proto__permission_query__descriptor,
      mumble_proto__codec_version__descriptor,
      mumble_proto__user_stats__descriptor,
      mumble_proto__request_blob__descriptor,
      mumble_proto__server_config__descriptor,
      mumble_proto__suggest_config__descriptor,
    };
    _descriptors = malloc(sizeof(tdescriptors));
    memcpy(_descriptors, tdescriptors, sizeof(tdescriptors));
  }

  return _descriptors;
}


int mumble_frame_init(mumble_frame_decoder_t *ctx) {
  ctx->buffer = malloc(MUMBLE_FRAME_MAX_SIZE);
  assert(ctx->buffer != NULL);
  ctx->buffer_allocated = MUMBLE_FRAME_MAX_SIZE;
  ctx->buffer_size = 0;
}

void mumble_frame_free(mumble_frame_decoder_t *ctx) {
  if (ctx->buffer != NULL) {
    free(ctx->buffer);
    ctx->buffer = NULL;
    ctx->buffer_size = 0;
    ctx->buffer_allocated = 0;
  }
}

int mumble_frame_size(mumble_frame_decoder_t *ctx) {
  if (ctx->buffer_size < sizeof(uint16_t) + sizeof(uint32_t)) {
    return -1;
  }

  uint32_t *size = (uint32_t*)(ctx->buffer + sizeof(uint16_t));
  return ntohl(*size);
}

int mumble_frame_type(mumble_frame_decoder_t *ctx) {
  if (ctx->buffer_size < sizeof(uint16_t)) {
    return -1;
  }

  return ntohs(*(uint16_t*)ctx->buffer);
}

int mumble_frame_ready(mumble_frame_decoder_t *ctx) {
  return mumble_frame_size(ctx) + MUMBLE_FRAME_HEADER_SIZE <= ctx->buffer_size;
}

int mumble_frame_is_audio(mumble_frame_decoder_t *ctx) {
  return mumble_frame_type(ctx) == MUMBLE_TYPE_UDPTUNNEL;
}

int mumble_frame_append(mumble_frame_decoder_t *ctx, const char* buf, unsigned size) {
  memcpy(ctx->buffer + ctx->buffer_size, buf, size);
  ctx->buffer_size += size;

  return mumble_frame_ready(ctx);
}

int mumble_frame_pop(mumble_frame_decoder_t *ctx) {
  if (!mumble_frame_ready(ctx)) {
    return -1;
  }

  int size = mumble_frame_size(ctx) + MUMBLE_FRAME_HEADER_SIZE;
  int new_size = ctx->buffer_size - size;
  memmove(ctx->buffer, ctx->buffer + size, new_size);
  ctx->buffer_size = new_size;

  return mumble_frame_ready(ctx);
}

int mumble_frame_decode(mumble_frame_decoder_t *ctx, ProtobufCMessage **message) {
  if (!mumble_frame_ready(ctx)) {
    return -1;
  }

  if (mumble_frame_is_audio(ctx)) {
    return -2;
  }

  int type = mumble_frame_type(ctx);
  ProtobufCMessageDescriptor descriptor = descriptors()[type];
  *message = protobuf_c_message_unpack(&descriptor, NULL, mumble_frame_size(ctx), (const uint8_t*)ctx->buffer + MUMBLE_FRAME_HEADER_SIZE);

  if (message == NULL) {
    return -3;
  }

  mumble_frame_pop(ctx);
  return type;
}
