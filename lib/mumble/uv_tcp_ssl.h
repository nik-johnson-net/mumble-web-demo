#ifndef _MUMBLE_UV_SSL_H_
#define _MUMBLE_UV_SSL_H_

#include <openssl/ssl.h>
#include <uv.h>

#include "util/buffer_pool.h"

struct _uv_tcp_ssl_t;

#define MUMBLE_CONNECTED 0
#define MUMBLE_FAILED 1

typedef void (*mumble_uv_connect_cb)(struct _uv_tcp_ssl_t *socket, int status);
typedef void (*mumble_uv_read_cb)(struct _uv_tcp_ssl_t *socket, int status, const void* buf, int size);

struct _uv_tcp_ssl_t {
  SSL *ssl;
  uv_tcp_t tcp;
  mumble_uv_read_cb cb;
  void* data;
  buffer_pool_t buffer_pool;
};

typedef struct _uv_tcp_ssl_t uv_tcp_ssl_t;

void mumble_uv_ssl_init(uv_tcp_ssl_t *socket);
void mumble_uv_ssl_close(uv_tcp_ssl_t *socket);
void mumble_uv_ssl_connect(uv_tcp_ssl_t *socket, const char* hostname, const char* port, mumble_uv_connect_cb cb);
void mumble_uv_ssl_free(uv_tcp_ssl_t *socket);
void mumble_uv_ssl_peername(const uv_tcp_ssl_t *socket, struct sockaddr *name, int *namelen);
void mumble_uv_ssl_set_data(uv_tcp_ssl_t *socket, void *data);
void mumble_uv_ssl_set_cb(uv_tcp_ssl_t *socket, mumble_uv_read_cb cb);
int mumble_uv_ssl_write(const uv_tcp_ssl_t *socket, const void* buf, int size);

#endif
