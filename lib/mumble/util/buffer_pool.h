#ifndef _UTIL_BUFFER_POOL_H_
#define _UTIL_BUFFER_POOL_H_

typedef struct {
  char **buffers;
  int in_use;
  unsigned size;
  unsigned buffer_count;
} buffer_pool_t;

void buffer_pool_init(buffer_pool_t *pool, unsigned number_of_buffers, unsigned buffer_size);
void buffer_pool_free(buffer_pool_t *pool);
char* buffer_pool_acquire(buffer_pool_t *pool);
void buffer_pool_release(buffer_pool_t *pool, char* buf);

#endif
