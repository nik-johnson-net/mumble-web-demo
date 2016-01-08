#include "audio_decoder.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void mumble_audio_decoder_init(mumble_audio_decoder_t *decoder) {
  memset(decoder, 0, sizeof(mumble_audio_decoder_t));
  decoder->opus = mumble_codec_opus_create();
  assert(decoder->opus != NULL);
}

int mumble_audio_decoder_decode(mumble_audio_decoder_t *decoder, const char *buf, int len, audio_packet_t *audio) {
  decoder->packets++;
  assert(len > 8);

  memset(audio, 0, sizeof(*audio));

  /* Codec */
  int type = MUMBLE_AUDIO_TYPE(*(uint8_t*)buf);
  assert(type != MUMBLE_AUDIO_PING);
  audio->codec = type;

  /* Target */
  audio->target = MUMBLE_AUDIO_TARGET(*(uint8_t*)buf);
  buf += sizeof(uint8_t);
  len -= sizeof(uint8_t);

  /* Source */
  uint64_t sourceid;
  int ret = varint_decode(buf, len, &sourceid);
  assert(ret > 0);
  audio->source = sourceid;
  buf += ret;
  len -= ret;

  /* Sequence */
  uint64_t seq;
  ret = varint_decode(buf, len, &seq);
  assert(ret > 0);
  audio->sequence = seq;
  buf += ret;
  len -= ret;

  if (seq < decoder->sequence) {
    decoder->dropped_sequence++;
    return 1;
  } else {
    decoder->sequence = seq;
  }

  /* Decode */
  int terminated;
  int pcm_size;
  if (type == MUMBLE_AUDIO_OPUS) {
    ret = mumble_codec_opus_decode(decoder->opus, buf, len, &audio->pcm.data, &pcm_size, &terminated);
    assert(ret > 0);
    buf += ret;
    len -= ret;

    audio->pcm.samples = ret;
    audio->pcm.hz = 48000;
  }

  /* Positional audio */
  if (len > 0) {
    assert(len == sizeof(float)*3);
    float *pos_buf = (float*)buf;
    audio->position.x = pos_buf[0];
    audio->position.y = pos_buf[1];
    audio->position.z = pos_buf[2];
  }

  /* Callback */
  if (decoder->cb.cb != NULL) {
    decoder->cb.cb(decoder, decoder->cb.data, audio);
  }
  return 0;
}

void mumble_audio_decoder_set_cb(mumble_audio_decoder_t *decoder, mumble_decoder_cb cb, void *data) {
  decoder->cb.cb = cb;
  decoder->cb.data = data;
}
