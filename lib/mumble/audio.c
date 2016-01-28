#include "audio.h"

#include <assert.h>
#include "util.h"

static void udp_cb(uv_udp_ssl_t *conn, void *data, char *buf, size_t len) {
  mumble_audio_t *audio = (mumble_audio_t*)data;
  audio_packet_t decoded;

  if (*buf == 0x20) {
    // If it's a ping packet, parse and notify pinger
    uint64_t timestamp;
    int ret = varint_decode(buf + 1, len - 1, &timestamp);
    assert(ret > 0);

    mumble_udp_ping_recv(&audio->pinger, timestamp);
  } else {
    // If it's an audio packet, fwd to decoder
    mumble_audio_decoder_decode(&audio->decoder, buf, len, &decoded);
  }
}

static void decoder_cb(mumble_audio_decoder_t *decoder, void *data, const audio_packet_t *decoded) {
  mumble_audio_t *audio = (mumble_audio_t*)data;

  if (audio->cb.cb != NULL) {
    audio->cb.cb(audio, audio->cb.data, decoded);
  }
}

static void encoder_cb(mumble_audio_encoder_t *encoder, void *data, const char *buf, unsigned size) {
  mumble_audio_t *audio = (mumble_audio_t*)data;

  if (audio->pinger.connected) {
    // Send to UDP
    uv_udp_ssl_write(&audio->udp,(struct sockaddr*)&audio->addr, buf, size);
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

    mumble_uv_ssl_write(audio->tcp, tcp_buf, length);
    free(tcp_buf);
  }
}

void mumble_audio_encryption(mumble_audio_t *audio, const char *key, const char *enc_iv, const char *dec_iv, unsigned ivlen) {
  uv_udp_ssl_set_encryption(&audio->udp, key, enc_iv, dec_iv, ivlen);
  mumble_udp_ping_start(&audio->pinger);
}

void mumble_audio_init(mumble_audio_t *audio, const uv_tcp_ssl_t *tcp, uv_loop_t *loop) {
  memset(audio, 0, sizeof(mumble_audio_t));

  audio->tcp = tcp;

  uv_udp_ssl_init(&audio->udp, loop);
  uv_udp_ssl_set_cb(&audio->udp, udp_cb, audio);

  mumble_audio_encoder_init(&audio->encoder, 48000, 20, 0);
  mumble_audio_encoder_set_cb(&audio->encoder, encoder_cb, audio);

  mumble_audio_decoder_init(&audio->decoder);
  mumble_audio_decoder_set_cb(&audio->decoder, decoder_cb, audio);

  mumble_udp_ping_init(&audio->pinger, &audio->udp, loop);
}

int mumble_audio_is_udp(mumble_audio_t *audio) {
  return audio->pinger.connected;
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

void mumble_audio_start(mumble_audio_t *audio) {
  int peer_len = sizeof(struct sockaddr_storage);
  mumble_uv_ssl_peername(audio->tcp, (struct sockaddr*)&audio->addr, &peer_len);
  audio->peer_addr_len = peer_len;

  mumble_udp_ping_address(&audio->pinger, &audio->addr);
}

void mumble_audio_stop(mumble_audio_t *audio) {
  mumble_udp_ping_stop(&audio->pinger);
}
