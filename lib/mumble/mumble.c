#include "mumble.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <openssl/ssl.h>
#include <uv.h>


typedef struct {
  uint16_t type;
  uint32_t length;
  void* payload;
} mumble_packet_t;

void mumble_client_init(mumble_client_t *client, const char *hostname, uint16_t port, const char* nick) {
  client->hostname = hostname;
  client->port = port;
  client->nick = nick;
  mumble_uv_ssl_init(&client->socket);
}

void mumble_client_connect(mumble_client_t *client) {
  char port_str[6] = {'\0'};
  snprintf(port_str, 6, "%d", client->port);
  mumble_uv_ssl_connect(&client->socket, client->hostname, port_str, NULL);
}

void mumble_client_on_message(mumble_client_t *client, int type, void *payload) {
}
