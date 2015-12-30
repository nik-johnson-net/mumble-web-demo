#include <stdio.h>
#include <string.h>

#include <mumble/mumble.h>
#include <mumble/proto/Mumble.pb-c.h>

void on_message(struct _mumble_client_t* client, int type, ProtobufCMessage *message) {
  switch (type) {
    case MUMBLE_TYPE_VERSION:
      ;
      MumbleProto__Version *version = (MumbleProto__Version*)message;
      int major = version->version >> 16;
      int minor = (version->version >> 8) & 0xFF;
      int bugfix = version->version & 0xFF;
      printf("Server version: %d.%d.%d (%s)\n", major, minor, bugfix, version->release);
      break;
  }
}

int main(int argc, char **argv) {
  SSL_library_init();

  mumble_client_t mclient;
  mumble_client_init(&mclient, "jnstw.us", 64738, "testbot");
  mumble_client_connect(&mclient);
  mumble_client_set_on_message(&mclient, on_message);

  fprintf(stderr, "Starting...\n");
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  fprintf(stderr, "Closing...\n");
  uv_loop_close(uv_default_loop());
  return 0;
}
