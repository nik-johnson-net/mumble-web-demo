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

void mumble_init(void) {
  SSL_library_init();
}

void mumble_client_init(mumble_client_t *client, const char *hostname, uint16_t port, const char* nick) {

  client->ssl = mumble_uv_ssl_init();
  client->hostname = hostname;
  client->port = port;
  client->nick = nick;
  int ret = uv_tcp_init(uv_default_loop(), &client->conn);
  client->conn.data = client;
  assert(ret == 0);
}

void default_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}

void mumble_client_do_ssl_handshake(mumble_client_t *client);
void ssl_handshake_continue(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  fprintf(stderr, "Read: %d\n", nread);
  if (nread > 0) {
    mumble_client_t *client = (mumble_client_t*)stream->data;
    mumble_client_do_ssl_handshake(client);
    free(buf->base);
  }
}

void mumble_client_do_ssl_handshake(mumble_client_t *client) {
  int ret = SSL_connect(client->ssl);
  fprintf(stderr, "SSL connect %d: ", ret);
  fprintf(stderr, "%s\n", SSL_state_string_long(client->ssl));
  switch (SSL_get_error(client->ssl, ret)) {
    case SSL_ERROR_NONE:
      fprintf(stderr, "SSL connect ok\n");
      break;
    case SSL_ERROR_ZERO_RETURN:
      fprintf(stderr, "SSL connect zero ret\n");
      break;
    case SSL_ERROR_WANT_CONNECT:
      fprintf(stderr, "SSL connect wants connection\n");
      break;
    case SSL_ERROR_WANT_X509_LOOKUP:
      fprintf(stderr, "SSL connect x509 lookup\n");
      break;
    case SSL_ERROR_SYSCALL:
      fprintf(stderr, "SSL connect syscall\n");
      break;
    case SSL_ERROR_SSL:
      fprintf(stderr, "SSL connect ssl error\n");
      break;
    case SSL_ERROR_WANT_WRITE:
      fprintf(stderr, "SSL connect want write\n");
      break;
    case SSL_ERROR_WANT_READ:
      fprintf(stderr, "SSL connect want read\n");
      int rret = uv_read_start((uv_stream_t*)&client->conn, default_alloc_cb, ssl_handshake_continue);
      assert(rret == 0);
      break;
    case SSL_ERROR_WANT_ACCEPT:
      fprintf(stderr, "SSL connect want accept\n");
      break;
    default:
      fprintf(stderr, "SSL connect %d\n", ret);
      break;
  }
}

void mumble_client_post_connect(uv_connect_t* req, int status) {
  fprintf(stderr, "Connected %d\n", status);
  mumble_client_t *client = (mumble_client_t*)req->data;
  uv_os_fd_t fd;
  int ret = uv_fileno((uv_handle_t*)req->handle, &fd);
  fprintf(stderr, "fileno %d\n", ret);
  assert(ret == 0);

  ret = SSL_set_fd(client->ssl, fd);
  fprintf(stderr, "SSL set %d\n", ret);
  assert(ret == 1);

  mumble_client_do_ssl_handshake(client);
}

void mumble_client_post_dns(uv_getaddrinfo_t* req_dns, int status, struct addrinfo* res) {
  fprintf(stderr, "DNS reoslved %d\n", status);
  switch (status) {
    case UV_ECANCELED:
      break;
  }
  uv_connect_t req;
  req.data = (mumble_client_t*)req_dns->data;
  mumble_client_t *client = req.data;
  fprintf(stderr, "Connecting\n");
  int ret = uv_tcp_connect(&req, &client->conn, res->ai_addr, &mumble_client_post_connect);
  fprintf(stderr, "uv_tcp_connect call %d\n", ret);

  uv_freeaddrinfo(res);
  free(req_dns);
}

void mumble_client_connect(mumble_client_t *client) {
  uv_getaddrinfo_t *req = (uv_getaddrinfo_t*)malloc(sizeof(uv_getaddrinfo_t));
  req->data = client;
  fprintf(stderr, "Looking up %s\n", client->hostname);
  struct addrinfo hints;
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0 ;
  //int ret = uv_getaddrinfo(uv_default_loop(), req, mumble_client_post_dns, client->hostname, "64738", &hints);
  int ret = uv_getaddrinfo(uv_default_loop(), req, mumble_client_post_dns, client->hostname, "443", &hints);
  assert(ret == 0);
  fprintf(stderr, "Getaddrinfo call %d\n", ret);
}
