#include "buffer_pool.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

void buffer_pool_init(buffer_pool_t *pool, unsigned number_of_buffers, unsigned buffer_size) {
  pool->buffer_count = number_of_buffers;
  pool->in_use = 0;
  pool->size = buffer_size;

  pool->buffers = malloc(number_of_buffers * sizeof(void*));
  assert(pool->buffers != NULL);
  for (int i = 0; i < number_of_buffers; i++) {
    pool->buffers[i] = malloc(buffer_size);
    assert(pool->buffers[i] != NULL);
  }
}

void buffer_pool_free(buffer_pool_t *pool) {
  for (int i = 0; i < pool->buffer_count; i++) {
    free(pool->buffers[i]);
  }
  free(pool->buffers);
}

char* buffer_pool_acquire(buffer_pool_t *pool) {
  char *next_buf = NULL;
  if (pool->in_use < pool->buffer_count) {
    next_buf = pool->buffers[pool->in_use];
    pool->in_use++;
  } else {
    fprintf(stderr, "WARNING: Buffer pool unable to acquire buffer.\n");
  }
  return next_buf;
}

void buffer_pool_release(buffer_pool_t *pool, char* buf) {
  for (int i = 0; i < pool->buffer_count; i++) {
    if (pool->buffers[i] == buf) {
      assert(pool->in_use > 0);

      pool->buffers[i] = pool->buffers[pool->in_use - 1];
      pool->buffers[pool->in_use - 1] = buf;
      pool->in_use--;
      break;
    }
  }
}
