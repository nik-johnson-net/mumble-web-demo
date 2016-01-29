#include "frame_encoder.h"

#include "common.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MUMBLE_FRAME_MAX_SIZE (1<<20)

static const ProtobufCMessageDescriptor descriptor(unsigned type) {
  ProtobufCMessageDescriptor descriptors[] = {
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

  assert(type < sizeof(descriptors));
  return descriptors[type];
}

static int get_message_type(ProtobufCMessage *message) {
  const ProtobufCMessageDescriptor *msg_descriptor = message->descriptor;
  assert(strcmp(msg_descriptor->package_name, "MumbleProto") == 0);

  int type = -1;
  for (int i = 0; i < MUMBLE_TYPE_COUNT; i++) {
    if (strcmp(msg_descriptor->name, descriptor(i).name) == 0) {
      type = i;
      break;
    }
  }

  return type;
}

int mumble_frame_encoder_init(mumble_frame_encoder_t *ctx) {
  ctx->buffer = malloc(MUMBLE_FRAME_MAX_SIZE);
  assert(ctx->buffer != NULL);
  ctx->buffer_allocated = MUMBLE_FRAME_MAX_SIZE;
  ctx->buffer_size = 0;
}

void mumble_frame_encoder_free(mumble_frame_encoder_t *ctx) {
  if (ctx->buffer != NULL) {
    free(ctx->buffer);
    ctx->buffer = NULL;
    ctx->buffer_size = 0;
    ctx->buffer_allocated = 0;
  }
}

void mumble_frame_encode(mumble_frame_encoder_t *ctx, ProtobufCMessage *message) {
  const ProtobufCMessageDescriptor *descriptor = message->descriptor;
  int type = get_message_type(message);
  assert(type != -1);

  size_t size = protobuf_c_message_get_packed_size(message);
  size_t total_size = sizeof(uint16_t) + sizeof(uint32_t) + size;
  assert(ctx->buffer_allocated >= total_size);

  *(uint16_t*)ctx->buffer = htons(type);
  *(uint32_t*)(ctx->buffer + sizeof(uint16_t)) = htonl(size);
  protobuf_c_message_pack(message, ctx->buffer + sizeof(uint16_t) + sizeof(uint32_t));

  ctx->buffer_size = total_size;
}
