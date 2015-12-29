#include "mumble.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <openssl/ssl.h>
#include <uv.h>
#include <arpa/inet.h>


typedef struct {
  uint16_t type;
  uint32_t length;
  const char* payload;
} mumble_packet_t;

static void mumble_message_cb_adapter(tcp_ssl_t *socket, int status, const void* buf, int size) {
  printf("Received\n");
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

    printf("Type: %d - len: %d\n", p.type, p.length);

    ptr += p.length;
  }

}

static void mumble_connect_cb_adapter(tcp_ssl_t *socket, int status) {
  printf("Connected.\n");

  char version[] = {
    // Header
    0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
    // Version
    0x09, 0x85, 0x82, 0x81, 0x00,
  };
  mumble_uv_ssl_write(socket, version, sizeof(version));

  char auth[] = {
    0x00, 0x02, 0x00, 0x00, 0x00, 0x07,
    // Username
    0x0a, 0x05, 't', 'e', 's', 't', 'r',
  };
  mumble_uv_ssl_write(socket, auth, sizeof(auth));
}

void mumble_client_init(mumble_client_t *client, const char *hostname, uint16_t port, const char* nick) {
  client->hostname = hostname;
  client->port = port;
  client->nick = nick;
  mumble_uv_ssl_init(&client->socket);
  mumble_uv_ssl_set_cb(&client->socket, mumble_message_cb_adapter);
}

void mumble_client_connect(mumble_client_t *client) {
  char port_str[6] = {'\0'};
  snprintf(port_str, 6, "%d", client->port);
  mumble_uv_ssl_connect(&client->socket, client->hostname, port_str, mumble_connect_cb_adapter);
}

void mumble_client_on_message(mumble_client_t *client, int type, void *payload) {
}
