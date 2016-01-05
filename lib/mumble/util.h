#ifndef _MUMBLE_UTIL_H_
#define _MUMBLE_UTIL_H_

#include <stdint.h>

int varint_encode_size(uint64_t source);
int varint_encode(uint64_t source, char *buf, int size, int *written);

int varint_decode(const uint8_t *buf, int size, uint64_t *result);

char* dupstr(const char* source);

#endif
