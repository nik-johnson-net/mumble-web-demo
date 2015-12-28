#ifndef _MUMBLE_H_
#define _MUMBLE_H_

#include <stdint.h>

#include <openssl/ssl.h>
#include <uv.h>

typedef struct {
  const char *hostname;
  uint16_t port;
  const char *nick;
  SSL *ssl;
  uv_tcp_t conn;
} mumble_client_t;

void mumble_init(void);
void mumble_client_init(mumble_client_t *client, const char *hostname, uint16_t port, const char* nick);
void mumble_client_connect(mumble_client_t *client);

#endif
