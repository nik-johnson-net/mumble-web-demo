#include "audio_encoder.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "util.h"

typedef struct {
  mumble_audio_encoder_t *encoder;
  audio_packet_t packet;
} encoder_codec_cb_t;

static void encoder_codec_cb(void *data, const char *buf, unsigned size, int bool_final) {
  encoder_codec_cb_t *encoder = (encoder_codec_cb_t*)data;
  encoder->packet.sequence = encoder->encoder->sequence++;
  
  // Allocate Buffer
  int encoded_size = size + (sizeof(uint64_t)*3) + sizeof(position_t) + sizeof(uint8_t);
  char *encoded_data = malloc(encoded_size);
  assert(encoded_data != NULL);
  int position = 0;

  // Write header
  *encoded_data = (encoder->packet.codec << 5) | (encoder->packet.target & 0x1F);
  position++;

  // Write sequence
  int written;
  int ret = varint_encode(encoder->packet.sequence, encoded_data + position, encoded_size, &written);
  assert(ret == 0);
  position += written;

  // Write payload
  assert(encoded_size - position > size);
  memcpy(encoded_data + position, buf, size);
  position += size;

  // Write position
  assert(encoded_size - position > sizeof(position_t));
  memcpy(encoded_data + position, &encoder->packet.position, sizeof(position_t));

  // fire callback
  encoder->encoder->cb.cb(encoder->encoder, encoder->encoder->cb.data, encoded_data, position);
  free(encoded_data);
}

void mumble_audio_encoder_init(mumble_audio_encoder_t *encoder, unsigned short sample_rate, unsigned short buffer_ms, int app) {
  memset(encoder, 0, sizeof(mumble_audio_encoder_t));

  encoder->opus = mumble_codec_opus_create_encoder(sample_rate, buffer_ms, app);
}

void mumble_audio_encoder_encode(mumble_audio_encoder_t *encoder, unsigned target, const pcm_t *pcm, const position_t *position) {
  encoder_codec_cb_t cb_t;
  cb_t.packet.codec = MUMBLE_AUDIO_OPUS;
  cb_t.encoder = encoder;
  cb_t.packet.target = target;

  if (position != NULL) {
    cb_t.packet.position = *position;
  }

  mumble_codec_opus_encode(encoder->opus, pcm->data, pcm->samples, 0, encoder_codec_cb, &cb_t);
}

void mumble_audio_encoder_set_cb(mumble_audio_encoder_t *encoder, mumble_encoder_cb cb, void *data) {
  encoder->cb.cb = cb;
  encoder->cb.data = data;
}
