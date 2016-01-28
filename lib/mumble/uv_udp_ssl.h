#ifndef _MUMBLE_UV_UDP_SSL_H_
#define _MUMBLE_UV_UDP_SSL_H_

#include <uv.h>
#include "ocb_aes.h"
#include "util/buffer_pool.h"

struct _uv_udp_ssl_t;

typedef void (*uv_udp_ssl_cb)(struct _uv_udp_ssl_t *conn, void *data, char *buff, size_t len);

struct uv_udp_ssl_cb_t {
  uv_udp_ssl_cb cb;
  void *data;
};

struct _uv_udp_ssl_t {
  uv_udp_t socket;
  ocb_aes_t cipher;
  struct uv_udp_ssl_cb_t cb;
  buffer_pool_t buffer_pool;
};

typedef struct _uv_udp_ssl_t uv_udp_ssl_t;

void uv_udp_ssl_init(uv_udp_ssl_t *conn);
void uv_udp_ssl_set_cb(uv_udp_ssl_t *conn, uv_udp_ssl_cb cb, void *data);
void uv_udp_ssl_set_encryption(uv_udp_ssl_t *conn, const char *key, const char *enc_iv, const char *dec_iv, unsigned ivlen);
void uv_udp_ssl_write(uv_udp_ssl_t *conn, const struct sockaddr *addr, const char *buf, size_t len);

#endif
