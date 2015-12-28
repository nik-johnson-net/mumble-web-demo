#include <assert.h>
#include <stdio.h>

#include "uv_ssl.h"

static void uv_ssl_handshake(tcp_ssl_t *socket);
static void uv_ssl_write_cb(uv_write_t* req, int status);

/* Called after libuv receives data on the socket.
 * Either finishes the handshake or calls the user cb
 */
static long uv_ssl_rbio_cb(BIO *b, int oper, const char *argp, int argi, long argl, long retvalue) {
  tcp_ssl_t *socket = (tcp_ssl_t*)BIO_get_callback_arg(b);
  switch (oper) {
    case BIO_CB_WRITE:
      fprintf(stderr, "Write from network into input buffer\n");
      // Input from UV
      fprintf(stderr, "Calling SSL_do_handshake\n");
      uv_ssl_handshake(socket);
      break;
    case BIO_CB_READ:
      // Output to SSL
      break;
    default:
      break;
  }
  return retvalue;
}


/* Called when libssl wants to send data to the socket.
 */
static long uv_ssl_wbio_cb(BIO *b, int oper, const char *argp, int argi, long argl, long retvalue) {
  tcp_ssl_t *socket = (tcp_ssl_t*)BIO_get_callback_arg(b);
  switch (oper) {
    case BIO_CB_WRITE:
      fprintf(stderr, "Write into output buffer\n");
      // Input from SSL
      uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
      char* arg = malloc(argi);
      memcpy(arg, argp, argi);
      uv_buf_t buf = uv_buf_init(arg, argi);
      fprintf(stderr, "Writing from output buffer to network\n");
      int ret = uv_write(req, (uv_stream_t*)&socket->tcp, &buf, 1, uv_ssl_write_cb);
      assert(ret == 0);
      break;
    case BIO_CB_READ:
      // Output to UV
      break;
    default:
      break;
  }
  return retvalue;
}

/* Initializes a socket with a libuv tcp handle and proper SSL objects
 */
void mumble_uv_ssl_init(tcp_ssl_t *socket) {
  int ret = uv_tcp_init(uv_default_loop(), &socket->tcp);
  socket->tcp.data = socket;
  assert(ret == 0);

  SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv23_client_method());
  assert(ssl_ctx != NULL);

  SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv3);
  SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
  socket->ssl = SSL_new(ssl_ctx);
  SSL_CTX_free(ssl_ctx);
  assert(socket->ssl != NULL);

  BIO *rbio = BIO_new(BIO_s_mem());
  BIO_set_callback(rbio, uv_ssl_rbio_cb);
  BIO_set_callback_arg(rbio, (char*)socket);

  BIO *wbio = BIO_new(BIO_s_null());
  BIO_set_callback(wbio, uv_ssl_wbio_cb);
  BIO_set_callback_arg(wbio, (char*)socket);

  SSL_set_bio(socket->ssl, rbio, wbio);
  SSL_set_connect_state(socket->ssl);
}

static void default_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}

static void uv_ssl_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  fprintf(stderr, "uv_ssl_read_cb: %u\n", nread);
  if (nread > 0) {
    fprintf(stderr, "uv_ssl_read_cb copying to input\n");
    tcp_ssl_t *client = (tcp_ssl_t*)stream->data;
    BIO *rbio = SSL_get_rbio(client->ssl);
    assert(rbio != NULL);
    BIO_write(rbio, buf->base, buf->len);
    free(buf->base);
  }
}

static void uv_ssl_write_cb(uv_write_t* req, int status) {
  fprintf(stderr, "write cb\n");
  assert(status == 0);
  free(req);
}

static void uv_ssl_handshake(tcp_ssl_t *socket) {
  int ret = SSL_do_handshake(socket->ssl);
  int req = SSL_get_error(socket->ssl, ret);
  switch (req) {
    default:
      fprintf(stderr, "SSL handshake %d\n", req);
      break;
  }
}

static void uv_ssl_connect_cb(uv_connect_t* req, int status) {
  assert(status == 0);
  fprintf(stderr, "Connected %d\n", status);
  
  tcp_ssl_t* socket = (tcp_ssl_t*)req->data;
  int ret = uv_read_start((uv_stream_t*)&socket->tcp, default_alloc_cb, uv_ssl_read_cb);
  assert (ret == 0);

  uv_ssl_handshake(socket);
}

static void uv_ssl_dns_cb(uv_getaddrinfo_t* req_dns, int status, struct addrinfo* res) {
  assert(status == 0);
  fprintf(stderr, "Connecting\n");

  uv_connect_t req;
  req.data = req_dns->data;
  tcp_ssl_t* socket = (tcp_ssl_t*)req.data;

  int ret = uv_tcp_connect(&req, &socket->tcp, res->ai_addr, uv_ssl_connect_cb);
  assert(ret == 0);

  uv_freeaddrinfo(res);
  free(req_dns);
}

void mumble_uv_ssl_connect(tcp_ssl_t *socket, const char* hostname, const char* port) {
  fprintf(stderr, "Looking up %s\n", hostname);
  uv_getaddrinfo_t *req = (uv_getaddrinfo_t*)malloc(sizeof(uv_getaddrinfo_t));
  req->data = socket;

  struct addrinfo hints;
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  int ret = uv_getaddrinfo(uv_default_loop(), req, uv_ssl_dns_cb, hostname, port, &hints);
  assert(ret == 0);
}

int mumble_uv_ssl_write(tcp_ssl_t *socket, uv_buf_t bufs[], int nbufs) {
  uv_write_t *req = (uv_write_t*)malloc(sizeof(uv_write_t));
  return uv_write(req, (uv_stream_t*)&socket->tcp, bufs, nbufs, NULL);
}
