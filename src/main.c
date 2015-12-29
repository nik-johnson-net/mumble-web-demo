#include <mumble.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  SSL_library_init();

  mumble_client_t mclient;
  mumble_client_init(&mclient, "jnstw.us", 64738, "testbot");
  mumble_client_connect(&mclient);

  fprintf(stderr, "Starting...\n");
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  fprintf(stderr, "Closing...\n");
  uv_loop_close(uv_default_loop());
  return 0;
}
