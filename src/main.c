#include <uv_ssl.h>
#include <stdio.h>

void ssl_cb(tcp_ssl_t *socket, int status) {
  switch (status) {
    case MUMBLE_CONNECTED:
      fprintf(stderr, "Connected!\n");
      break;
    case MUMBLE_FAILED:
      fprintf(stderr, "Couldn't connect :(\n");
      break;
  }
}

int main(int argc, char **argv) {
  SSL_library_init();

  tcp_ssl_t socket;
  mumble_uv_ssl_init(&socket);
  mumble_uv_ssl_connect(&socket, "twitter.com", "443", ssl_cb);

  fprintf(stderr, "Starting...\n");
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  fprintf(stderr, "Closing...\n");
  uv_loop_close(uv_default_loop());
  return 0;
}
