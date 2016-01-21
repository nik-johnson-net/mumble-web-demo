#include "uv_udp_ssl.h"

#include <assert.h>
#include <openssl/evp.h>
#include <string.h>
#include "util.h"

struct decrypted {
  char *buf;
  size_t len;
};

static void increment_iv(unsigned char *iv, unsigned len) {
  for (int i = 0; i < len; i++) {
    iv[i]++;
    if (iv[i] != 0) {
      break;
    }
  }
}

static void uv_udp_ssl_decrypt(uv_udp_ssl_t *conn, char *buf, size_t len) {
  unsigned char iv;
  unsigned char tag[3];
  unsigned char intag[16];
  unsigned char out[10240];

  increment_iv(conn->cipher.dec_iv, conn->cipher.ivlen);
  iv = buf[0];
  tag[0] = buf[1];
  tag[1] = buf[2];
  tag[2] = buf[3];

  ocb2_aes_decrypt(&conn->cipher, buf+4, len-4, out, intag);

  if (conn->cb.cb) {
    conn->cb.cb(conn, conn->cb.data, out, len - 4);
  }
}

static void recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
  uv_udp_ssl_t *conn = handle->data;
  if (nread > 0) {
    uv_udp_ssl_decrypt(conn, buf->base, nread);
  }
  free(buf->base);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = malloc(suggested_size);
  buf->len = buf->base == NULL ? 0 : suggested_size;
}

void uv_udp_ssl_init(uv_udp_ssl_t *conn) {
  memset(conn, 0, sizeof(uv_udp_ssl_t));

  uv_udp_init(uv_default_loop(), &conn->socket);
  uv_udp_recv_start(&conn->socket, alloc_cb, recv_cb);
  conn->socket.data = conn;
}

void uv_udp_ssl_set_cb(uv_udp_ssl_t *conn, uv_udp_ssl_cb cb, void *data) {
  conn->cb.cb = cb;
  conn->cb.data = data;
}

void uv_udp_ssl_set_encryption(uv_udp_ssl_t *conn, const char *key, const char *enc_iv, const char *dec_iv, unsigned ivlen) {
  ocb_aes_set_keys(&conn->cipher, key, enc_iv, dec_iv, ivlen);
}

static int encrypt_ocb(uv_udp_ssl_t *conn, const char *source, char *dest, size_t len, char *tag) {
  increment_iv(conn->cipher.enc_iv, conn->cipher.ivlen);
  ocb2_aes_encrypt(&conn->cipher, source, len, dest, tag);
  return len;
}

void uv_udp_ssl_write(uv_udp_ssl_t *conn, const struct sockaddr *addr, const char *buf, size_t len) {
  uv_buf_t send_buffer;
  send_buffer.base = conn->out_buffer;
  send_buffer.len = MAX_UDP_SIZE;
  char tag[16];

  int out_size = encrypt_ocb(conn, buf, send_buffer.base + 4, len, tag);
  send_buffer.base[0] = conn->cipher.enc_iv[0];
  send_buffer.base[1] = tag[0];
  send_buffer.base[2] = tag[1];
  send_buffer.base[3] = tag[2];

  /* Send chunk out */
  send_buffer.len = out_size + 4;
  int ret = uv_udp_try_send(&conn->socket, &send_buffer, 1, addr);
  assert(ret > 0);

  // TODO: free
}
