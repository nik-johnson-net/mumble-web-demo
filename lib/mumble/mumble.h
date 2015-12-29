#ifndef _MUMBLE_H_
#define _MUMBLE_H_

#include <stdint.h>
#include "uv_ssl.h"

struct _mumble_client_t;

typedef struct {
  uint16_t type;
  uint32_t length;
  const char* payload;
} mumble_packet_t;

typedef void (*mumble_client_on_message)(struct _mumble_client_t* client, const mumble_packet_t *packet);

struct _mumble_client_t {
  const char *hostname;
  uint16_t port;
  const char *nick;
  tcp_ssl_t socket;
  mumble_client_on_message on_message;
};
typedef struct _mumble_client_t mumble_client_t;

void mumble_client_init(mumble_client_t *client, const char *hostname, uint16_t port, const char* nick);
void mumble_client_connect(mumble_client_t *client);
void mumble_client_set_on_message(mumble_client_t *client, mumble_client_on_message cb);

#endif
