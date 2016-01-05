#include <stdio.h>
#include <string.h>

#include <mumble/mumble.h>
#include <mumble/proto/Mumble.pb-c.h>

void on_message(struct _mumble_client_t* client, void *data, int type, ProtobufCMessage *message) {
  switch (type) {
    case MUMBLE_TYPE_VERSION:
      ;
      MumbleProto__Version *version = (MumbleProto__Version*)message;
      int major = version->version >> 16;
      int minor = (version->version >> 8) & 0xFF;
      int bugfix = version->version & 0xFF;
      printf("Server version: %d.%d.%d (%s)\n", major, minor, bugfix, version->release);
      break;
    case MUMBLE_TYPE_TEXT_MESSAGE:
      ;
      MumbleProto__TextMessage *msg = (MumbleProto__TextMessage*)message;
      printf("Text: [%u] %s\n", msg->actor, msg->message);
      break;
  }
}

void on_audio(mumble_client_t *client, void *data, const audio_decoded_t *audio) {
  printf("Received audio packet from %u (%u) (%u) (Codec: %d)\n  Length: %d frames\n", audio->source, audio->target, audio->sequence, audio->codec, audio->pcm.samples);
}

int main(int argc, char **argv) {
  SSL_library_init();

  mumble_client_t mclient;
  mumble_client_init(&mclient, "jnstw.us", 64738, "testbot");
  mumble_client_connect(&mclient);
  mumble_client_set_on_message(&mclient, on_message, NULL);
  mumble_client_set_on_audio(&mclient, on_audio, NULL);

  fprintf(stderr, "Starting...\n");
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  fprintf(stderr, "Closing...\n");
  uv_loop_close(uv_default_loop());
  return 0;
}
