#ifndef _MUMBLE_AUDIO_H_
#define _MUMBLE_AUDIO_H_

#include "uv_udp_ssl.h"
#include "uv_tcp_ssl.h"
#include "audio_decoder.h"
#include "audio_encoder.h"

struct _mumble_audio_t;

typedef void (*mumble_audio_on_audio_cb)(struct _mumble_audio_t *audio, void *data, const audio_decoded_t *decoded);

typedef struct {
  mumble_audio_on_audio_cb cb;
  void *data;
} mumble_audio_on_audio_cb_t;

struct _mumble_audio_t {
  int udp_connected;
  uv_udp_ssl_t udp;
  const uv_tcp_ssl_t *tcp;
  const char *address;
  unsigned short port;
  mumble_audio_on_audio_cb_t cb;
  mumble_audio_decoder_t decoder;
  mumble_audio_encoder_t encoder;
};

typedef struct _mumble_audio_t mumble_audio_t;

/* Set UDP channel encryption params */
void mumble_audio_encryption(mumble_audio_t *audio, const char *key, const char *enc_iv, const char *dec_iv);

/* Init the interface */
void mumble_audio_init(mumble_audio_t *audio, const uv_tcp_ssl_t *tcp, const char *address, unsigned short port);

/* Returns 1 if UDP is being used */
int mumble_audio_is_udp(mumble_audio_t *audio);

/* Returns the decoder to inject mumble audio frames */
mumble_audio_decoder_t* mumble_audio_decoder(mumble_audio_t *audio);

/* Send  PCM audio */
void mumble_audio_send(mumble_audio_t *audio, int target, const pcm_t *pcm);

/* Set the callback for when audio is received */
void mumble_audio_set_cb(mumble_audio_t *audio, mumble_audio_on_audio_cb cb, void *data);

#endif
