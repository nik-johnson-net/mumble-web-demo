#include "opus.h"

#define DEFAULT_Fs 48000

#include <assert.h>
#include <opus/opus.h>
#include <stdlib.h>
#include <string.h>

#include "../buffer.h"
#include "../util.h"


typedef struct _codec_opus_decoder_t {
  OpusDecoder *decoder;
} codec_opus_decoder_t;

struct _mumble_codec_cb_t {
  mumble_codec_cb cb;
  void *data;
};

typedef struct {
  char *buffer;
  int size;
} _codec_buf;

static void codec_buf_init(_codec_buf *buf, int size) {
  buf->buffer = malloc(size);
  assert(buf->buffer != NULL);
  buf->size = size;
}

typedef struct _codec_opus_encoder_t {
  OpusEncoder *encoder;
  mumble_buffer_t *buffer;
  unsigned short buffer_ms;
  _codec_buf codec_out;
  _codec_buf framed_out;
  struct _mumble_codec_cb_t cb;
} codec_opus_encoder_t;

codec_opus_decoder_t *mumble_codec_opus_create(void) {
  codec_opus_decoder_t *codec = malloc(sizeof(codec_opus_decoder_t));
  assert(codec != NULL);
  memset(codec, 0, sizeof(codec_opus_decoder_t));

  int error;
  codec->decoder = opus_decoder_create(DEFAULT_Fs, 1, &error);
  assert(error == OPUS_OK);
  assert(codec->decoder != NULL);

  return codec;
}

int mumble_codec_opus_decode(codec_opus_decoder_t *codec, const char *buffer, int len, uint16_t **pcm, int *pcm_len, int *termination) {
  *termination = 0;
  uint64_t varlen;
  int ret = varint_decode(buffer, len, &varlen);

  // Check termination
  if (varlen & 0x2000) {
    varlen = varlen & 0x1FFF;
    *termination = 1;
  }

  assert(ret > 0);
  assert(varlen > 0);
  assert(varlen < len);

  buffer += ret;
  len -= ret;
  assert(varlen == len);

  int num_samples = opus_decoder_get_nb_samples(codec->decoder, buffer, varlen);
  *pcm_len = sizeof(opus_int16) * num_samples;
  *pcm = malloc(*pcm_len);
  assert(*pcm != NULL);

  ret = opus_decode(codec->decoder, buffer, varlen, *pcm, *pcm_len, 0);
  assert(ret > 0);

  return ret;
}

static void on_encoder_frame(void *data, const char *buf, unsigned size, int bool_final) {
  codec_opus_encoder_t *codec = (codec_opus_encoder_t*)data;

  int ret = opus_encode(
      codec->encoder,
      (const uint16_t*)buf,
      size/sizeof(uint16_t),
      codec->codec_out.buffer,
      codec->codec_out.size);
  assert(ret > 0);

  int written;
  int varint_ret = varint_encode(ret, codec->framed_out.buffer, codec->framed_out.size, &written);
  assert(varint_ret == 0);
  assert(codec->framed_out.size > ret + written);
  memcpy(codec->framed_out.buffer + written, codec->codec_out.buffer, ret);
  codec->cb.cb(codec->cb.data, codec->framed_out.buffer, written + ret, bool_final);
}

codec_opus_encoder_t *mumble_codec_opus_create_encoder(unsigned short sample_rate, unsigned short buffer_ms, int app) {
  // Set default
  if (app == 0) {
    app = OPUS_APPLICATION_VOIP;
  }

  codec_opus_encoder_t *codec = malloc(sizeof(codec_opus_encoder_t));
  assert(codec != NULL);
  memset(codec, 0, sizeof(codec_opus_encoder_t));

  int error;
  codec->encoder = opus_encoder_create(sample_rate, 1, app, &error);
  assert(error == OPUS_OK);
  assert(codec->encoder != NULL);

  int audio_buffer_size = sample_rate * ((float)buffer_ms / 1000);
  codec->buffer_ms = buffer_ms;
  codec->buffer = mumble_buffer_create(audio_buffer_size, on_encoder_frame, codec);

  codec_buf_init(&codec->codec_out, audio_buffer_size);
  codec_buf_init(&codec->framed_out, audio_buffer_size);

  return codec;
}

int mumble_codec_opus_encode(codec_opus_encoder_t *codec, const uint16_t *pcm, int pcm_size, int termination, mumble_codec_cb cb, void *cb_data) {
  codec->cb.cb = cb;
  codec->cb.data = cb_data;

  mumble_buffer_write(codec->buffer, (const char*)pcm, pcm_size*sizeof(uint16_t), termination);
}
