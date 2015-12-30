#ifndef _MUMBLE_H_
#define _MUMBLE_H_

#include <stdint.h>
#include "uv_ssl.h"

#include "proto/Mumble.pb-c.h"

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


struct _mumble_client_t;

typedef struct {
  uint16_t type;
  uint32_t length;
  const char* payload;
} mumble_packet_t;

typedef void (*mumble_client_on_message)(struct _mumble_client_t* client, int type, ProtobufCMessage *message);

struct _mumble_client_t {
  const char *hostname;
  uint16_t port;
  const char *nick;
  tcp_ssl_t socket;
  uv_timer_t ping_timer;
  mumble_client_on_message on_message;
};
typedef struct _mumble_client_t mumble_client_t;

void mumble_client_init(mumble_client_t *client, const char *hostname, uint16_t port, const char* nick);
void mumble_client_connect(mumble_client_t *client);
void mumble_client_set_on_message(mumble_client_t *client, mumble_client_on_message cb);
void mumble_client_write(mumble_client_t *client, ProtobufCMessage *message);

#endif
