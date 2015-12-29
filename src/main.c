#include <uv_ssl.h>
#include <stdio.h>
#include <string.h>

const char *req = "GET / HTTP/1.1\r\nHost: twitter.com\r\nConnection: close\r\n\r\n";

void on_data(tcp_ssl_t *socket, int status, char *buf, int size) {
  // printf("status: %d (%d)\n", status, size);
  switch (status) {
    case MUMBLE_CONNECTED:
      if (buf[size-1] != '\0') {
        // printf("appending null\n");
        buf = realloc(buf, size + 1);
        buf[size] = '\0';
      }
      printf("%s", buf);
      break;
    case MUMBLE_FAILED:
      fprintf(stderr, "Broken :(\n");
      break;
  }
}

void ssl_cb(tcp_ssl_t *socket, int status) {
  switch (status) {
    case MUMBLE_CONNECTED:
      fprintf(stderr, "Connected!\n");
      mumble_uv_ssl_write(socket, req, strlen(req));
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
  mumble_uv_ssl_set_cb(&socket, on_data);
  mumble_uv_ssl_connect(&socket, "twitter.com", "443", ssl_cb);

  fprintf(stderr, "Starting...\n");
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  fprintf(stderr, "Closing...\n");
  uv_loop_close(uv_default_loop());
  return 0;
}
