#ifndef MUMBLE_CODEC_OPUS_H_
#define MUMBLE_CODEC_OPUS_H_

#include <stdint.h>

typedef struct _codec_opus_t codec_opus_t;

codec_opus_t *mumble_codec_opus_create(void);
int mumble_codec_opus_decode(codec_opus_t *codec, const char *buffer, int len, uint16_t **pcm, int *pcm_size, int *termination);

#endif
