#ifndef _MUMBLE_H_
#define _MUMBLE_H_

#include <stdint.h>

#include "audio.h"
#include "frame_decoder.h"
#include "uv_tcp_ssl.h"

#include "proto/Mumble.pb-c.h"


struct _mumble_client_t;

typedef struct {
  uint16_t type;
  uint32_t length;
  const char* payload;
} mumble_packet_t;

typedef void (*mumble_client_on_message)(struct _mumble_client_t* client, void *data, int type, ProtobufCMessage *message);
typedef void (*mumble_client_on_audio)(struct _mumble_client_t* client, void *data, const audio_packet_t *audio);

typedef struct {
  mumble_client_on_message cb;
  void *data;
} mumble_client_on_message_t;

typedef struct {
  mumble_client_on_audio cb;
  void *data;
} mumble_client_on_audio_t;

struct _mumble_client_t {
  const char *hostname;
  uint16_t port;
  const char *nick;
  uv_tcp_ssl_t socket;
  uv_timer_t ping_timer;

  mumble_audio_t audio;
  mumble_client_on_message_t on_message;
  mumble_client_on_audio_t on_audio;
  mumble_frame_decoder_t decoder;
};
typedef struct _mumble_client_t mumble_client_t;

/* Initialize a mumble connect with required args */
void mumble_client_init(mumble_client_t *client, const char *hostname, uint16_t port, const char* nick);

/* Connect to the server */
void mumble_client_connect(mumble_client_t *client);

/* Set the callback to receive audio */
void mumble_client_set_on_audio(mumble_client_t *client, mumble_client_on_audio cb, void *data);

/* Set the callback to receive control messages */
void mumble_client_set_on_message(mumble_client_t *client, mumble_client_on_message cb, void *data);

/* Send a control message */
void mumble_client_write(mumble_client_t *client, ProtobufCMessage *message);

/* Send an audio packet. Buf contains PCM audio */
void mumble_client_write_audio(mumble_client_t *client, int target, pcm_t pcm);

#endif
