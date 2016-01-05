#include "audio.h"

#include <assert.h>
#include "util.h"

static void udp_cb(uv_udp_ssl_t *conn, void *data, char *buf, size_t len) {
  mumble_audio_t *audio = (mumble_audio_t*)data;
  audio_decoded_t decoded;
  mumble_audio_decoder_decode(&audio->decoder, buf, len, &decoded);
}

static void decoder_cb(mumble_audio_decoder_t *decoder, void *data, const audio_decoded_t *decoded) {
  mumble_audio_t *audio = (mumble_audio_t*)data;

  if (audio->cb.cb != NULL) {
    audio->cb.cb(audio, audio->cb.data, decoded);
  }
}

static void encoder_cb(mumble_audio_encoder_t *encoder, void *data, const audio_encoded_t *encoded) {
  mumble_audio_t *audio = (mumble_audio_t*)data;

  // Serialize

  if (audio->udp_connected) {
    // Send to UDP
  } else {
    // Send to TCP
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

  mumble_audio_encoder_init(&audio->encoder);
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
  mumble_audio_encoder_encode(&audio->encoder, pcm);
}

void mumble_audio_set_cb(mumble_audio_t *audio, mumble_audio_on_audio_cb cb, void *data) {
  audio->cb.cb = cb;
  audio->cb.data = data;
}
