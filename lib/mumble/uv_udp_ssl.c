#include "uv_udp_ssl.h"

#include <assert.h>
#include <openssl/evp.h>
#include <string.h>
#include "util.h"

#define DECRYPTING 0
#define ENCRYPTING 1

struct decrypted {
  char *buf;
  size_t len;
};

static void init_ssl_cipher(EVP_CIPHER_CTX *a, unsigned char *key, unsigned char *iv, int enc) {
  EVP_CIPHER_CTX_init(a);
  EVP_CipherInit_ex(a, EVP_aes_128_ofb(), NULL, key, iv, enc);
}

static void free_if_allocated(char **str) {
  assert(str != NULL);

  if (*str != NULL) {
    free(*str);
    *str = NULL;
  }
}

static void uv_udp_ssl_decrypt(uv_udp_ssl_t *conn, char *buf, size_t len) {
  EVP_CIPHER_CTX dec;
  init_ssl_cipher(&dec, conn->key, conn->dec_iv, DECRYPTING);
  EVP_CIPHER_CTX_cleanup(&dec);
}

void uv_udp_ssl_init(uv_udp_ssl_t *conn) {
  memset(conn, 0, sizeof(uv_udp_ssl_t));

  uv_udp_init(uv_default_loop(), &conn->socket);
}

void uv_udp_ssl_clean(uv_udp_ssl_t *conn) {
  free_if_allocated(&conn->key);
  free_if_allocated(&conn->enc_iv);
  free_if_allocated(&conn->dec_iv);
}

void uv_udp_ssl_set_cb(uv_udp_ssl_t *conn, uv_udp_ssl_cb cb, void *data) {
  conn->cb.cb = cb;
  conn->cb.data = data;
}

void uv_udp_ssl_set_encryption(uv_udp_ssl_t *conn, const char *key, const char *enc_iv, const char *dec_iv) {
  uv_udp_ssl_clean(conn);

  conn->key = dupstr(key);
  conn->enc_iv = dupstr(enc_iv);
  conn->dec_iv = dupstr(dec_iv);

  assert(conn->key);
  assert(conn->enc_iv);
  assert(conn->dec_iv);
}

void uv_udp_ssl_write(uv_udp_ssl_t *conn, const struct sockaddr *addr, const char *buf, size_t len) {
  uv_buf_t send_buffer;

  EVP_CIPHER_CTX enc;
  init_ssl_cipher(&enc, conn->key, conn->enc_iv, ENCRYPTING);

  /* allocate enough room */
  send_buffer.base = malloc(send_buffer.len);
  assert(send_buffer.base != NULL);

  /* build cipher text */
  int ret = EVP_EncryptUpdate(&enc, (unsigned char*)&send_buffer.base, &send_buffer.len, buf, len);

  /* Output padded tail */
  int final_out = 0;
  ret = EVP_EncryptFinal(&enc, (unsigned char*)&send_buffer.base + send_buffer.len, &final_out);
  send_buffer.len += final_out;

  /* The cipher isn't needed anymore */
  EVP_CIPHER_CTX_cleanup(&enc);

  /* Send chunk out */
  uv_udp_send_t req;
  ret = uv_udp_send(&req, &conn->socket, &send_buffer, 1, addr, NULL);
}
