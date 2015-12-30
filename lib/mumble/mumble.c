#include "mumble.h"
#include "proto/Mumble.pb-c.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <openssl/ssl.h>
#include <uv.h>
#include <arpa/inet.h>

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

static void mumble_message_parse(mumble_client_t *client, const mumble_packet_t *p) {
  ProtobufCMessage *message = NULL;
  ProtobufCMessageDescriptor descriptor;

  if (p->type < MUMBLE_TYPE_COUNT) {
    descriptor = descriptors()[p->type];
  }

  message = protobuf_c_message_unpack(&descriptor, NULL, p->length, (const uint8_t*)p->payload);

  if (message != NULL && client->on_message != NULL) {
    client->on_message(client, p->type, message);
  }
}

static void mumble_message_cb_adapter(tcp_ssl_t *socket, int status, const void* buf, int size) {
  mumble_client_t *client = (mumble_client_t*)socket->data;
  const char* ptr = buf;
  const char* end = buf + size;
  while (ptr < end) {
    mumble_packet_t p;

    p.type = ntohs(*(uint16_t*)ptr);
    ptr += sizeof(uint16_t);
    assert(ptr < end);

    p.length = ntohl(*(uint32_t*)ptr);
    ptr += sizeof(uint32_t);
    assert(ptr < end);

    p.payload = ptr;
    ptr += p.length;
    assert(ptr <= end);

    mumble_message_parse(client, &p);
  }
}

static void mumble_client_on_ping(uv_timer_t *handle) {
  mumble_client_t *client = (mumble_client_t*)handle->data;

  MumbleProto__Ping ping = MUMBLE_PROTO__PING__INIT;
  ping.timestamp = (int)time(NULL);

  mumble_client_write(client, (ProtobufCMessage*)&ping);
}

static void mumble_connect_cb_adapter(tcp_ssl_t *socket, int status) {
  mumble_client_t *client = (mumble_client_t*)socket->data;
  uv_timer_start(&client->ping_timer, mumble_client_on_ping, 15000, 30000);

  MumbleProto__Version version = MUMBLE_PROTO__VERSION__INIT;
  version.version = (1 << 16) | (2 << 8) | 5;
  mumble_client_write(client, (ProtobufCMessage*)&version);

  MumbleProto__Authenticate authenticate = MUMBLE_PROTO__AUTHENTICATE__INIT;
  authenticate.username = client->nick;
  mumble_client_write(client, (ProtobufCMessage*)&authenticate);

  MumbleProto__Ping ping = MUMBLE_PROTO__PING__INIT;
  ping.timestamp = (int)time(NULL);
  mumble_client_write(client, (ProtobufCMessage*)&ping);
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
}

void mumble_client_connect(mumble_client_t *client) {
  char port_str[6] = {'\0'};
  snprintf(port_str, 6, "%d", client->port);
  mumble_uv_ssl_connect(&client->socket, client->hostname, port_str, mumble_connect_cb_adapter);
}

void mumble_client_set_on_message(mumble_client_t *client, mumble_client_on_message cb) {
  client->on_message = cb;
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
