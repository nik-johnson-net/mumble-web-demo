#include "util.h"

#include <stdlib.h>
#include <string.h>

char* dupstr(const char* source) {
  int len = strlen(source) + 1;
  char *dest = malloc(len);
  if (dest == NULL) {
    return dest;
  }
  return strncpy(dest, source, len);
}

int varint_decode(const uint8_t *buf, int size, uint64_t *result) {
  if (size == 0) {
    return 0;
  }

  int position = 0;
  *result = 0;
  
  if  ((buf[position] & 0x80) == 0x00) {
    *result = (uint8_t)buf[position] & 0x7F;
  } else if ((buf[position] & 0xC0) == 0x80) {
    if (size < 2) {
      return -1;
    }
    *result = ((buf[position] & 0x3F) << 8) |  buf[++position];
  } else if ((buf[position] & 0xF0) == 0xF0) {
    switch (buf[position] & 0xFC) {
      case 0xF0:
        *result = buf[++position] << 24 | buf[++position] << 16 | buf[++position] << 8 | buf[++position];
        break;
      case 0xF4:
        *result = (uint64_t)buf[++position] << 56 | (uint64_t)buf[++position] << 48 | (uint64_t)buf[++position] << 40 | (uint64_t)buf[++position] << 32 | buf[++position] << 24 | buf[++position] << 16 | buf[++position] << 8 | buf[++position];
        break;
      case 0xF8:
        ;
        int r_result = varint_decode(buf + position, size - position, result);
        if (r_result != 0) {
          return r_result;
        }
        *result = ~*result;
        break;
      case 0xFC:
        *result = buf[position] & 0x03;
        *result = ~*result;
        break;
      default:
        return -2;
        break;
    }
  } else if ((buf[position] & 0xF0) == 0xE0) {
      *result = (buf[position] & 0x0F) << 24 | buf[++position] << 16 | buf[++position] << 8 | buf[++position];
  } else if ((buf[position] & 0xE0) == 0xC0) {
      *result = (buf[position] & 0x1F) << 16 | buf[++position] << 8 | buf[++position];
  }
  
  return position + 1;
}

int varint_encode_size(uint64_t source) {
  int size = 0;

  // Negative numbers represented by prepending negative marker
  if (source < 0) {
    source = ~source;
    size++;
    if (source < 0x4) {
      return size;
    }
  }

  if (source < 0x80) {
    size += 1;
  } else if (source < 0x4000) {
    size += 2;
  } else if (source < 0x200000) {
    size += 3;
  } else if (source < 0x10000000) {
    size += 4;
  } else if (source < 0x100000000LL) {
    size += 5;
  } else {
    size += 9;
  }

  return size;
}

static void write_buf(char *buf, int *pos, int *written, char value) {
  buf[(*pos)++] = value;
  (*written)++;
}

int varint_encode(uint64_t source, char *buf, int size, int *written) {
  int needed = varint_encode_size(source);
  if (size < needed) {
    return 1;
  }

  *written = 0;
  int pos = 0;

  if (source < 0) {
    source = ~source;
    if (source < 0x4) {
      write_buf(buf, &pos, written, 0xFC | source);
      return 0;
    } else {
      write_buf(buf, &pos, written, 0xF8);
    }
  }

  
  if (source < 0x80) {
    // Need top bit clear
    write_buf(buf, &pos, written, source);
  } else if (source < 0x4000) {
    // Need top two bits clear
    write_buf(buf, &pos, written, (source >> 8) | 0x80);
    write_buf(buf, &pos, written, source & 0xFF);
  } else if (source < 0x200000) {
    // Need top three bits clear
    write_buf(buf, &pos, written, (source >> 16) | 0xC0);
    write_buf(buf, &pos, written, (source >> 8) & 0xFF);
    write_buf(buf, &pos, written, source & 0xFF);
  } else if (source < 0x10000000) {
    // Need top four bits clear
    write_buf(buf, &pos, written, (source >> 24) | 0xE0);
    write_buf(buf, &pos, written, (source >> 16) & 0xFF);
    write_buf(buf, &pos, written, (source >> 8) & 0xFF);
    write_buf(buf, &pos, written, source & 0xFF);
  } else if (source < 0x100000000LL) {
    // It's a full 32-bit integer.
    write_buf(buf, &pos, written, 0xF0);
    write_buf(buf, &pos, written, (source >> 24) & 0xFF);
    write_buf(buf, &pos, written, (source >> 16) & 0xFF);
    write_buf(buf, &pos, written, (source >> 8) & 0xFF);
    write_buf(buf, &pos, written, source & 0xFF);
  } else {
    // It's a 64-bit value.
    write_buf(buf, &pos, written, 0xF4);
    write_buf(buf, &pos, written, (source >> 56) & 0xFF);
    write_buf(buf, &pos, written, (source >> 48) & 0xFF);
    write_buf(buf, &pos, written, (source >> 40) & 0xFF);
    write_buf(buf, &pos, written, (source >> 32) & 0xFF);
    write_buf(buf, &pos, written, (source >> 24) & 0xFF);
    write_buf(buf, &pos, written, (source >> 16) & 0xFF);
    write_buf(buf, &pos, written, (source >> 8) & 0xFF);
    write_buf(buf, &pos, written, source & 0xFF);
  }
  return 0;
}
