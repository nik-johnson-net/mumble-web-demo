#include "buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a,b) (a < b ? a : b)

struct _mumble_buffer_cb_t {
  mumble_buffer_cb cb;
  void *data;
};

struct _mumble_buffer_t {
  char *buf;
  unsigned buf_allocated;
  unsigned buf_used;
  struct _mumble_buffer_cb_t cb;
};

mumble_buffer_t *mumble_buffer_create(unsigned size, mumble_buffer_cb cb, void *cb_data) {
  mumble_buffer_t *buf = malloc(sizeof(mumble_buffer_t));
  assert(buf != NULL);

  memset(buf, 0, sizeof(mumble_buffer_t));

  buf->buf = malloc(size);
  assert(buf->buf != NULL);

  buf->buf_allocated = size;
  buf->cb.cb = cb;
  buf->cb.data = cb_data;

  return buf;
}

void mumble_buffer_flush(mumble_buffer_t *buffer) {
  memset(buffer->buf + buffer->buf_used, 0, buffer->buf_allocated - buffer->buf_used);
  buffer->buf_used = buffer->buf_allocated;
  mumble_buffer_write(buffer, NULL, 0, 1);
}

void mumble_buffer_free(mumble_buffer_t *buffer) {
  free(buffer->buf);
  free(buffer);
}

void mumble_buffer_write(mumble_buffer_t *buffer, const char *buf, unsigned size, int end) {
  const char *incoming_ptr = buf;
  const char *end_ptr = buf + size;

  while (incoming_ptr < end_ptr) {
    unsigned amount_to_copy = MIN(end_ptr - incoming_ptr, buffer->buf_allocated - buffer->buf_used);
    memcpy(buffer->buf + buffer->buf_used, incoming_ptr, amount_to_copy);
    buffer->buf_used += amount_to_copy;
    incoming_ptr += amount_to_copy;

    if (buffer->buf_used == buffer->buf_allocated) {
      int final_write = 0;
      if (end) {
        final_write = incoming_ptr == end_ptr;
      }

      /* Target size reached, notify callback */
      buffer->cb.cb(buffer->cb.data, buffer->buf, buffer->buf_used, final_write);

      /* Empty the buffer */
      buffer->buf_used = 0;
    }
  }

  /* If this should be a final write but there's still data left over, flush */
  if (end && buffer->buf_used > 0) {
    mumble_buffer_flush(buffer);
  }
}
