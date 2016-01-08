#ifndef MUMBLE_CODEC_OPUS_H_
#define MUMBLE_CODEC_OPUS_H_

#include <stdint.h>

typedef struct _codec_opus_decoder_t codec_opus_decoder_t;
typedef struct _codec_opus_encoder_t codec_opus_encoder_t;

typedef void (*mumble_codec_cb)(void *data, const char *buf, unsigned size, int bool_final);

codec_opus_decoder_t *mumble_codec_opus_create_decoder(void);
int mumble_codec_opus_decode(codec_opus_decoder_t *codec, const char *buffer, int len, uint16_t **pcm, int *pcm_size, int *termination);

codec_opus_encoder_t *mumble_codec_opus_create_encoder(unsigned short sample_rate, unsigned short buffer_ms, int app);
int mumble_codec_opus_encode(codec_opus_encoder_t *codec, const uint16_t *pcm, int samples, int termination, mumble_codec_cb cb, void *cb_data);

#endif
