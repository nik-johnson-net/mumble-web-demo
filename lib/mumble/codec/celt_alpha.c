#include "celt_alpha.h"

void write(char *payload) {
  int written;
  int header = varint_decode(payload, &written);

  char *data = payload + written;
}
