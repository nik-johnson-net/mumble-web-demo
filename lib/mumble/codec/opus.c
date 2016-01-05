#include "opus.h"

#define DEFAULT_Fs 48000

#include <assert.h>
#include <opus/opus.h>
#include <stdlib.h>
#include <string.h>
#include "../util.h"


typedef struct _codec_opus_t {
  OpusDecoder *decoder;
} codec_opus_t;

codec_opus_t *mumble_codec_opus_create(void) {
  codec_opus_t *codec = malloc(sizeof(codec_opus_t));
  assert(codec != NULL);
  memset(codec, 0, sizeof(codec_opus_t));

  int error;
  codec->decoder = opus_decoder_create(DEFAULT_Fs, 1, &error);
  assert(error == OPUS_OK);
  assert(codec->decoder != NULL);

  return codec;
}

int mumble_codec_opus_decode(codec_opus_t *codec, const char *buffer, int len, uint16_t **pcm, int *pcm_len, int *termination) {
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
