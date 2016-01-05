#include "audio_encoder.h"

#include <string.h>

void mumble_audio_encoder_init(mumble_audio_encoder_t *encoder) {
  memset(encoder, 0, sizeof(mumble_audio_encoder_t));
}

audio_encoded_t mumble_audio_encoder_encode(mumble_audio_encoder_t *encoder, const pcm_t *pcm) {
  audio_encoded_t audio;

  if (encoder->cb.cb != NULL) {
    encoder->cb.cb(encoder, encoder->cb.data, &audio);
  }

  return audio;
}

void mumble_audio_encoder_set_cb(mumble_audio_encoder_t *encoder, mumble_encoder_cb cb, void *data) {
  encoder->cb.cb = cb;
  encoder->cb.data = data;
}
