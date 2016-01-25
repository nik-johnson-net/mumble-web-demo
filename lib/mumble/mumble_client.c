#include "mumble_client.h"
#include "proto/Mumble.pb-c.h"
#include "util.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <openssl/ssl.h>
#include <uv.h>

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


static void mumble_message_internal(mumble_client_t *client, int type, ProtobufCMessage *message) {
  switch (type) {
    case MUMBLE_TYPE_UDPTUNNEL:
      {
        MumbleProto__UDPTunnel *msg = (MumbleProto__UDPTunnel*)message;
      }
      break;
    case MUMBLE_TYPE_CRYPT_SETUP:
      {
        MumbleProto__CryptSetup *msg = (MumbleProto__CryptSetup*)message;
        assert(msg->has_key);
        assert(msg->has_client_nonce);
        assert(msg->has_server_nonce);
        mumble_audio_encryption(&client->audio, msg->key.data, msg->client_nonce.data, msg->server_nonce.data, msg->client_nonce.len);
      }
      break;
  }
}

static void mumble_message_cb_adapter(uv_tcp_ssl_t *socket, int status, const void* buf, int size) {
  mumble_client_t *client = (mumble_client_t*)socket->data;
  mumble_frame_append(&client->decoder, buf, size);

  while (mumble_frame_ready(&client->decoder)) {
    if (mumble_frame_is_audio(&client->decoder)) {
      // Send to audio
      // TODO: Static decoder instance? Free?
      mumble_audio_decoder_t *decoder = mumble_audio_decoder(&client->audio);
      audio_packet_t decoded;
      mumble_audio_decoder_decode(decoder, client->decoder.buffer, client->decoder.buffer_size, &decoded);
      mumble_frame_pop(&client->decoder);

    } else {
      // Decode
      ProtobufCMessage *message;
      int type = mumble_frame_decode(&client->decoder, &message);

      if (type >= 0) {
        // If successful, run internal routines
        mumble_message_internal(client, type, message);

        // Run callback
        if (client->on_message.cb != NULL) {
          client->on_message.cb(client, client->on_message.data, type, message);
        }

        // TODO: Proper cleanup?
        free(message);
      }
    }
  }
}

static void mumble_client_on_ping(uv_timer_t *handle) {
  mumble_client_t *client = (mumble_client_t*)handle->data;

  MumbleProto__Ping ping = MUMBLE_PROTO__PING__INIT;
  ping.timestamp = (int)time(NULL);

  mumble_client_write(client, (ProtobufCMessage*)&ping);
}

static void mumble_connect_cb_adapter(uv_tcp_ssl_t *socket, int status) {
  mumble_client_t *client = (mumble_client_t*)socket->data;

  // Start audio subsystem
  mumble_audio_start(&client->audio);

  // Begin ping timer
  // TODO(jumpandspintowin): Split TCP Ping into subsystem
  uv_timer_start(&client->ping_timer, mumble_client_on_ping, 15000, 30000);

  // Send version info, lets the server know we accept the 1.2.5 protocol
  MumbleProto__Version version = MUMBLE_PROTO__VERSION__INIT;
  version.version = (1 << 16) | (2 << 8) | 5;
  mumble_client_write(client, (ProtobufCMessage*)&version);

  // Send Auth info. Tells the server the username and that opus is supported.
  MumbleProto__Authenticate authenticate = MUMBLE_PROTO__AUTHENTICATE__INIT;
  authenticate.username = dupstr(client->nick);
  authenticate.opus = 1;
  authenticate.has_opus = 1;
  mumble_client_write(client, (ProtobufCMessage*)&authenticate);

  // Send an initial ping
  MumbleProto__Ping ping = MUMBLE_PROTO__PING__INIT;
  ping.timestamp = (int)time(NULL);
  mumble_client_write(client, (ProtobufCMessage*)&ping);
}

static void mumble_on_audio(mumble_audio_t *audio, void *data, const audio_packet_t *decoded) {
  mumble_client_t *client = (mumble_client_t*)data;
  if (client->on_audio.cb != NULL) {
    client->on_audio.cb(client, client->on_audio.data, decoded);
  }
}

void mumble_client_init(mumble_client_t *client, const char *hostname, uint16_t port, const char* nick) {
  memset(client, 0, sizeof(mumble_client_t));

  client->hostname = hostname;
  client->port = port;
  client->nick = nick;

  mumble_uv_ssl_init(&client->socket);
  mumble_uv_ssl_set_cb(&client->socket, mumble_message_cb_adapter);
  mumble_uv_ssl_set_data(&client->socket, client);

  uv_timer_init(uv_default_loop(), &client->ping_timer);
  client->ping_timer.data = client;

  mumble_audio_init(&client->audio, &client->socket);
  mumble_audio_set_cb(&client->audio, mumble_on_audio, client);

  mumble_frame_init(&client->decoder);
}

void mumble_client_connect(mumble_client_t *client) {
  char port_str[6] = {'\0'};
  snprintf(port_str, 6, "%d", client->port);
  mumble_uv_ssl_connect(&client->socket, client->hostname, port_str, mumble_connect_cb_adapter);
}

void mumble_client_set_on_audio(mumble_client_t *client, mumble_client_on_audio cb, void *data) {
  client->on_audio.cb = cb;
  client->on_audio.data = data;
}

void mumble_client_set_on_message(mumble_client_t *client, mumble_client_on_message cb, void *data) {
  client->on_message.cb = cb;
  client->on_message.data = data;
}

static int get_message_type(ProtobufCMessage *message) {
  const ProtobufCMessageDescriptor *descriptor = message->descriptor;
  assert(strcmp(descriptor->package_name, "MumbleProto") == 0);

  int type = -1;
  for (int i = 0; i < MUMBLE_TYPE_COUNT; i++) {
    if (strcmp(descriptor->name, descriptors()[i].name) == 0) {
      type = i;
      break;
    }
  }

  return type;
}

void mumble_client_write(mumble_client_t *client, ProtobufCMessage *message) {
  const ProtobufCMessageDescriptor *descriptor = message->descriptor;
  int type = get_message_type(message);
  assert(type != -1);

  size_t size = protobuf_c_message_get_packed_size(message);
  size_t total_size = sizeof(uint16_t) + sizeof(uint32_t) + size;
  char* buffer = malloc(total_size);
  assert(buffer != NULL);

  *(uint16_t*)buffer = htons(type);
  *(uint32_t*)(buffer + sizeof(uint16_t)) = htonl(size);
  protobuf_c_message_pack(message, buffer + sizeof(uint16_t) + sizeof(uint32_t));

  mumble_uv_ssl_write(&client->socket, buffer, total_size);
}

void mumble_client_write_audio(mumble_client_t *client, int target, pcm_t pcm) {
  mumble_audio_send(&client->audio, target, &pcm);
}
