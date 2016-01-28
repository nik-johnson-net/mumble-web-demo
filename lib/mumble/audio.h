#ifndef _MUMBLE_AUDIO_H_
#define _MUMBLE_AUDIO_H_

#include "audio_decoder.h"
#include "audio_encoder.h"
#include "udp_ping.h"
#include "uv_tcp_ssl.h"
#include "uv_udp_ssl.h"

struct _mumble_audio_t;

typedef void (*mumble_audio_on_audio_cb)(struct _mumble_audio_t *audio, void *data, const audio_packet_t *decoded);

typedef struct {
  mumble_audio_on_audio_cb cb;
  void *data;
} mumble_audio_on_audio_cb_t;

struct _mumble_audio_t {
  uv_udp_ssl_t udp;
  const uv_tcp_ssl_t *tcp;
  mumble_audio_on_audio_cb_t cb;
  mumble_audio_decoder_t decoder;
  mumble_audio_encoder_t encoder;

  struct sockaddr_storage addr;
  int peer_addr_len;

  udp_ping_t pinger;
};

typedef struct _mumble_audio_t mumble_audio_t;

/* Set UDP channel encryption params */
void mumble_audio_encryption(mumble_audio_t *audio, const char *key, const char *enc_iv, const char *dec_iv, unsigned ivlen);

/* Init the interface */
void mumble_audio_init(mumble_audio_t *audio, const uv_tcp_ssl_t *tcp, uv_loop_t *loop);

/* Returns 1 if UDP is being used */
int mumble_audio_is_udp(mumble_audio_t *audio);

/* Returns the decoder to inject mumble audio frames */
mumble_audio_decoder_t* mumble_audio_decoder(mumble_audio_t *audio);

/* Send  PCM audio */
void mumble_audio_send(mumble_audio_t *audio, int target, const pcm_t *pcm);

/* Set the callback for when audio is received */
void mumble_audio_set_cb(mumble_audio_t *audio, mumble_audio_on_audio_cb cb, void *data);

/* Start audio subsystem */
void mumble_audio_start(mumble_audio_t *audio);

/* Stop audio subsystem */
void mumble_audio_stop(mumble_audio_t *audio);

#endif
