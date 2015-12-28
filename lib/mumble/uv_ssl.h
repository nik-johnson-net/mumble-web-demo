#ifndef _MUMBLE_UV_SSL_H_
#define _MUMBLE_UV_SSL_H_

#include <openssl/ssl.h>
#include <uv.h>

typedef struct {
  SSL *ssl;
  uv_tcp_t tcp;
} tcp_ssl_t;

void mumble_uv_ssl_init(tcp_ssl_t *socket);
void mumble_uv_ssl_connect(tcp_ssl_t *socket, const char* hostname, const char* port);
int mumble_uv_ssl_write(tcp_ssl_t *socket, uv_buf_t bufs[], int nbufs);

#endif
