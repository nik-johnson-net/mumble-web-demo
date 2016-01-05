#ifndef _MUMBLE_BUFFER_H_
#define _MUMBLE_BUFFER_H_

/* A Buffer which calls a callback with a constant frame size of data. Writes
 * can be of any size and the callback will only be invoked when the buffer
 * fills. A write with the end bit set and flush call will call the callback
 * with the final flag set. Flush and write_final will zero the rest of the
 * buffer.
 * */

typedef struct _mumble_buffer_t mumble_buffer_t;

typedef void (*mumble_buffer_cb)(void *data, const char *buf, unsigned size, int bool_final);

mumble_buffer_t *mumble_buffer_create(unsigned size, mumble_buffer_cb cb, void *cb_data);
void mumble_buffer_flush(mumble_buffer_t *buffer);
void mumble_buffer_free(mumble_buffer_t *buffer);
void mumble_buffer_write(mumble_buffer_t *buffer, const char *buf, unsigned size, int end);

#endif
