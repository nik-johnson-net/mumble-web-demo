#include <uv_ssl.h>
#include <stdio.h>

int main(int argc, char **argv) {
  SSL_library_init();

  tcp_ssl_t socket;
  mumble_uv_ssl_init(&socket);
  mumble_uv_ssl_connect(&socket, "google.com", "443");

  fprintf(stderr, "Starting...\n");
  int ret = uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  fprintf(stderr, "Closing... %d\n", ret);
  uv_loop_close(uv_default_loop());
  return 0;
}
