#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mumble/mumble_client.h>
#include <mumble/proto/Mumble.pb-c.h>

#define PI 3.14159265

uint16_t* tone(double time, int sample_rate, int frequency) {
  int samples = (time * sample_rate);
  uint16_t *tone = malloc(samples * sizeof(uint16_t));

  double rate_of_inc = ((2.0*PI * frequency) / sample_rate);

  for (int i = 0; i < samples; i++) {
    double sample = sin(rate_of_inc*i);

    // Normalize to 16bit
    uint16_t bit_sample = (sample + 1) * ((1 << 15) - 1);

    tone[i] = bit_sample;
  }

  return tone;
}

void send_tone(mumble_client_t *client, int frequency, double time) {
  pcm_t pcm;
  pcm.data = tone(time, 48000, frequency);
  pcm.hz = 48000;
  pcm.samples = time * 48000;

  mumble_client_write_audio(client, 0, pcm);
}

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

      send_tone(client, 300, 2);
      break;
  }
}

void on_audio(mumble_client_t *client, void *data, const audio_packet_t *audio) {
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
