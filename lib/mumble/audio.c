#include "audio.h"

#include <assert.h>
#include "util.h"

static void udp_cb(uv_udp_ssl_t *conn, void *data, char *buf, size_t len) {
  mumble_audio_t *audio = (mumble_audio_t*)data;
  audio_packet_t decoded;
  mumble_audio_decoder_decode(&audio->decoder, buf, len, &decoded);
}

static void decoder_cb(mumble_audio_decoder_t *decoder, void *data, const audio_packet_t *decoded) {
  mumble_audio_t *audio = (mumble_audio_t*)data;

  if (audio->cb.cb != NULL) {
    audio->cb.cb(audio, audio->cb.data, decoded);
  }
}

static void encoder_cb(mumble_audio_encoder_t *encoder, void *data, const char *buf, unsigned size) {
  mumble_audio_t *audio = (mumble_audio_t*)data;

  if (audio->udp_connected) {
    // Send to UDP
    assert(0);
  } else {
    // Add header
    int length = size + 6;
    char* tcp_buf = malloc(length);
    memset(tcp_buf, 0, length);
    assert(tcp_buf != NULL);

    short *type = (short*)tcp_buf;
    *type = htons(MUMBLE_TYPE_UDPTUNNEL);
    uint32_t *len = (uint32_t*)(tcp_buf + sizeof(short));
    *len = htonl(size);
    memcpy(tcp_buf + 6, buf, size);

    for (int i = 0; i < length; i++) {
      printf("%02x ", ((const unsigned char *) tcp_buf)[i] & 0xff);
    }
    printf("\n");

    printf("%d -> %d\n", size, length);
    mumble_uv_ssl_write(audio->tcp, tcp_buf, length);
    free(tcp_buf);
  }
}

void mumble_audio_encryption(mumble_audio_t *audio, const char *key, const char *enc_iv, const char *dec_iv) {
  uv_udp_ssl_set_encryption(&audio->udp, key, enc_iv, dec_iv);
}

void mumble_audio_init(mumble_audio_t *audio, const uv_tcp_ssl_t *tcp, const char *address, unsigned short port) {
  memset(audio, 0, sizeof(mumble_audio_t));

  audio->tcp = tcp;
  audio->address = dupstr(address);
  audio->port = port;

  uv_udp_ssl_init(&audio->udp);
  uv_udp_ssl_set_cb(&audio->udp, udp_cb, audio);

  mumble_audio_encoder_init(&audio->encoder, 48000, 20, 0);
  mumble_audio_encoder_set_cb(&audio->encoder, encoder_cb, audio);

  mumble_audio_decoder_init(&audio->decoder);
  mumble_audio_decoder_set_cb(&audio->decoder, decoder_cb, audio);

  assert(audio->address);
}

int mumble_audio_is_udp(mumble_audio_t *audio) {
  return audio->udp_connected;
}

mumble_audio_decoder_t* mumble_audio_decoder(mumble_audio_t *audio) {
  return &audio->decoder;
}

void mumble_audio_send(mumble_audio_t *audio, int target, const pcm_t *pcm) {
  mumble_audio_encoder_encode(&audio->encoder, target, pcm, NULL);
}

void mumble_audio_set_cb(mumble_audio_t *audio, mumble_audio_on_audio_cb cb, void *data) {
  audio->cb.cb = cb;
  audio->cb.data = data;
}
