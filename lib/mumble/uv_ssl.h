#ifndef _MUMBLE_UV_SSL_H_
#define _MUMBLE_UV_SSL_H_

#include <openssl/ssl.h>
#include <uv.h>

struct _tcp_ssl_t;

#define MUMBLE_CONNECTED 0
#define MUMBLE_FAILED 1

typedef void (*mumble_uv_connect_cb)(struct _tcp_ssl_t *socket, int status);
typedef void (*mumble_uv_read_cb)(struct _tcp_ssl_t *socket, int status, const void* buf, int size);

struct _tcp_ssl_t {
  SSL *ssl;
  uv_tcp_t tcp;
  mumble_uv_read_cb cb;
};

typedef struct _tcp_ssl_t tcp_ssl_t;

void mumble_uv_ssl_init(tcp_ssl_t *socket);
void mumble_uv_ssl_connect(tcp_ssl_t *socket, const char* hostname, const char* port, mumble_uv_connect_cb cb);
void mumble_uv_ssl_set_cb(tcp_ssl_t *socket, mumble_uv_read_cb cb);
int mumble_uv_ssl_write(tcp_ssl_t *socket, const void* buf, int size);

#endif
