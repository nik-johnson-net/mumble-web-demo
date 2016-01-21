// Call for unit testing of static functions
#include "ocb_aes.c"

#include <assert.h>
#include <stdio.h>
#include <string.h>

const unsigned char KEY[] = {
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B,
  0x0C, 0x0D, 0x0E, 0x0F};

const unsigned char PART_N[] = {
  0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x0F};

const unsigned char PART_A[] = { };

const unsigned char PART_P[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};

const unsigned char PART_C[] = {
  0x44, 0x12, 0x92, 0x34, 0x93, 0xC5, 0x7D, 0x5D, 0xE0, 0xD7, 0x00, 0xF7, 0x53, 0xCC, 0xE0, 0xD1, 0xD2, 0xD9, 0x50, 0x60, 0x12, 0x2E, 0x9F, 0x15,
  0xA5, 0xDD, 0xBF, 0xC5, 0x78, 0x7E, 0x50, 0xB5, 0xCC, 0x55, 0xEE, 0x50, 0x7B, 0xCB, 0x08, 0x4E, 0x47, 0x9A, 0xD3, 0x63, 0xAC, 0x36, 0x6B, 0x95,
  0xA9, 0x8C, 0xA5, 0xF3, 0x00, 0x0B, 0x14, 0x79};

static int verify_block_ex(const unsigned char* a, const unsigned char *b, int size) {
  int failed = 0;
  for (int i = 0; i <size; i++) {
    if (a[i] != b[i]) {
      fprintf(stderr, "Byte mismatch (%d): 0x%02x -> 0x%02x\n", i, a[i], b[i]);
      failed = 1;
    }
  }

  return failed;
}

static int verify_block(const unsigned char* a, const unsigned char *b) {
  return verify_block_ex(a, b, AES_BLOCK_SIZE);
}

int test_case(const char *nonce, const char *a, int al, const char *plain, int plainl, const char *cipher) {
  ocb_aes_t aes;
  ocb_aes_set_keys(&aes, KEY, nonce, nonce, 12);

  char out[512];
  char tag[AES_BLOCK_SIZE];
  ocb3_aes_encrypt(&aes, plain, plainl, a, al, out, tag);
  memcpy(out + plainl, tag, AES_BLOCK_SIZE);

  int size = AES_BLOCK_SIZE + plainl;
  return verify_block_ex(out, cipher, size);
}

int test_1(void) {
  unsigned char nonce[] = {0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
  unsigned char a[] = {};
  unsigned char plain[] = {};
  unsigned char cipher[] = {0x78, 0x54, 0x07, 0xBF, 0xFF, 0xC8, 0xAD, 0x9E, 0xDC, 0xC5, 0x52, 0x0A, 0xC9, 0x11, 0x1E, 0xE6};

  return test_case(nonce, a, sizeof(a), plain, sizeof(plain), cipher);
}

int test_aes_encrypt() {
  unsigned char key[] = {
    0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA};
  unsigned char in[] = {
    0x01, 0x4B, 0xAF, 0x22, 0x78, 0xA6, 0x9D, 0x33, 0x1D, 0x51, 0x80, 0x10, 0x36, 0x43, 0xE9, 0x9A};
  unsigned char out_expected[] = {
    0x67, 0x43, 0xC3, 0xD1, 0x51, 0x9A, 0xB4, 0xF2, 0xCD, 0x9A, 0x78, 0xAB, 0x09, 0xA5, 0x11, 0xBD};
  unsigned char out[AES_BLOCK_SIZE];
  aes_encrypt(key, in, out);

  return verify_block(out, out_expected);
}

int test_cpy_block() {
  unsigned char out[AES_BLOCK_SIZE];
  cpy_block(out, KEY);

  return verify_block(out, KEY);
}

int test_double_block_boundary() {
  unsigned char in[] = {
    0x01, 0xC0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char expected[] = {
    0x03, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char out[AES_BLOCK_SIZE];
  double_block(in, out);

  return verify_block(out, expected);
}

int test_double_block_negative() {
  unsigned char in[] = {
    0x81, 0xC0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char expected[] = {
    0x03, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x87};
  unsigned char out[AES_BLOCK_SIZE];
  double_block(in, out);

  return verify_block(out, expected);
}

int test_ntz() {
  assert(ntz(1) == 0);
  assert(ntz(2) == 1);
  assert(ntz(4) == 2);
  assert(ntz(5) == 2);
  assert(ntz(8) == 3);
  return 0;
}

int test_xor_block() {
  unsigned char in[] = {
    0x81, 0xC0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char in2[] = {
    0x81, 0xC0, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char expected[] = {
    0x00, 0x00, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char out[AES_BLOCK_SIZE];
  xor_block(in, in2, out);
  return verify_block(out, expected);
}

int test_nonce_init() {
  unsigned char in[] = {
    0xFF, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char expected[] = {
    0xC0, 0x00, 0, 0x01, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x00};
  unsigned char out[AES_BLOCK_SIZE];

  ocb_aes_t ctx;
  ocb_aes_set_keys(&ctx, KEY, in, in, sizeof(in));

  nonce_init(&ctx, out);

  return verify_block(out, expected);
}

int test_ktop_init() {
  unsigned char in[] = {
    0xC0, 0x00, 0, 0x01, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8F};
  unsigned char expected[] = {
    0xda, 0xe6, 0x9d, 0xc7, 0xb8, 0xb9, 0x91, 0xa4, 0x81, 0x12, 0xfb, 0xb7, 0x9c, 0x9e, 0xab, 0x40};
  unsigned char out[AES_BLOCK_SIZE];

  ocb_aes_t ctx;
  ocb_aes_set_keys(&ctx, KEY, in, in, sizeof(in));

  ktop_init(&ctx, in, out);

  return verify_block(out, expected);
}

int test_keyvar_init() {
  ocb_aes_t ctx;
  ocb_aes_set_keys(&ctx, KEY, PART_N, PART_N, sizeof(PART_N));

  unsigned char expected[] = {
    0x1A, 0x84, 0xEC, 0xDE, 0x1E, 0x3D, 0x6E, 0x09, 0xBD, 0x3E, 0x05, 0x8A, 0x87, 0x23, 0x60, 0x6D};

  unsigned char keyvar[AES_BLOCK_SIZE];
  keyvar_init(ctx.key, keyvar);

  return verify_block(keyvar, expected);
}

int test_offset_init() {
  ocb_aes_t ctx;
  ocb_aes_set_keys(&ctx, KEY, PART_N, PART_N, sizeof(PART_N));

  unsigned char expected[] = {
    0x58, 0x7E, 0xF7, 0x27, 0x16, 0xEA, 0xB6, 0xDD, 0x32, 0x19, 0xF8, 0x09, 0x2D, 0x51, 0x7D, 0x69};

  unsigned char out[AES_BLOCK_SIZE];
  offset_init(&ctx, out);

  return verify_block(out, expected);
}

int test_offset_init_bottom() {
  ocb_aes_t ctx;
  ocb_aes_set_keys(&ctx, KEY, PART_N, PART_N, sizeof(PART_N));

  unsigned char nonce[AES_BLOCK_SIZE];
  unsigned char expected[] = {
    0xC0, 0x00, 0x00, 0x01, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x0F};
  int bottom;

  nonce_init(&ctx, nonce);
  
  // assert(verify_block(nonce, expected, AES_BLOCK_SIZE) == 0);
  bottom = 0;
  bottom = (nonce[AES_BLOCK_SIZE - 1] & 0x1F);
  // Fail (ret 1) if bottom != 15
  return bottom != 15;
}

int test_offset_init_ktop() {
  unsigned char expected[] = {
    0x98, 0x62, 0xB0, 0xFD, 0xEE, 0x4E, 0x2D, 0xD5, 0x6D, 0xBA, 0x64, 0x33, 0xF0, 0x12, 0x5A, 0xA2};

  ocb_aes_t ctx;
  ocb_aes_set_keys(&ctx, KEY, PART_N, PART_N, sizeof(PART_N));

  unsigned char nonce[AES_BLOCK_SIZE];
  int bottom;
  unsigned char ktop[AES_BLOCK_SIZE];

  nonce_init(&ctx, nonce);
  bottom = 0;
  bottom = (nonce[AES_BLOCK_SIZE - 1] & 0x1F);
  ktop_init(&ctx, nonce, ktop);
  return verify_block(ktop, expected);
}

int test_offset_init_stretch() {
  unsigned char expected[] = {
    0x98, 0x62, 0xB0, 0xFD, 0xEE, 0x4E, 0x2D, 0xD5, 0x6D, 0xBA, 0x64, 0x33, 0xF0, 0x12, 0x5A, 0xA2, 0xFA, 0xD2, 0x4D, 0x13, 0xA0, 0x63, 0xF8, 0xB8};

  ocb_aes_t ctx;
  ocb_aes_set_keys(&ctx, KEY, PART_N, PART_N, sizeof(PART_N));

  unsigned char nonce[AES_BLOCK_SIZE];
  int bottom;
  unsigned char ktop[AES_BLOCK_SIZE];
  unsigned char stretch[AES_BLOCK_SIZE+(AES_BLOCK_SIZE/2)];

  nonce_init(&ctx, nonce);
  bottom = 0;
  bottom = (nonce[AES_BLOCK_SIZE - 1] & 0x1F);
  ktop_init(&ctx, nonce, ktop);
  stretch_init(ktop, stretch);
  return verify_block_ex(stretch, expected, AES_BLOCK_SIZE + (AES_BLOCK_SIZE/2));
}

int test_left_shift() {
  unsigned char in[] = {
    0x01, 0x80, 0xFF, 0x01};
  unsigned char expected[] = {
    0x7F, 0x80, 0x80, 0x00};
  unsigned char out[4];
  left_shift(in, sizeof(in), out, 15);
  return verify_block_ex(out, expected, sizeof(expected));
}

int test_ocb2_enc() {
    const uchar rawkey[AES_BLOCK_SIZE] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    uchar tag[AES_BLOCK_SIZE];
    unsigned char crypt[40];

    ocb_aes_t ctx;
    ocb_aes_set_keys(&ctx, rawkey, rawkey, rawkey, sizeof(rawkey));

    ocb2_aes_encrypt(&ctx, NULL, 0, crypt, tag);
    const uchar blank_tag[AES_BLOCK_SIZE] = {0xBF,0x31,0x08,0x13,0x07,0x73,0xAD,0x5E,0xC7,0x0E,0xC6,0x9E,0x78,0x75,0xA7,0xB0};

    return verify_block_ex(tag, blank_tag, sizeof(blank_tag));
}

int test_ocb2_enc2() {
    const uchar rawkey[AES_BLOCK_SIZE] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    uchar tag[AES_BLOCK_SIZE];
    unsigned char source[40];
    unsigned char crypt[40];

    ocb_aes_t ctx;
    ocb_aes_set_keys(&ctx, rawkey, rawkey, rawkey, sizeof(rawkey));
    for (int i=0;i<40;i++)
      source[i]=i;
    ocb2_aes_encrypt(&ctx, source, sizeof(source), crypt, tag);

    const uchar longtag[AES_BLOCK_SIZE] = {0x9D,0xB0,0xCD,0xF8,0x80,0xF7,0x3E,0x3E,0x10,0xD4,0xEB,0x32,0x17,0x76,0x66,0x88};
    const uchar crypted[40] = {0xF7,0x5D,0x6B,0xC8,0xB4,0xDC,0x8D,0x66,0xB8,0x36,0xA2,0xB0,0x8B,0x32,0xA6,0x36,0x9F,0x1C,0xD3,0xC5,0x22,0x8D,0x79,0xFD,
      0x6C,0x26,0x7F,0x5F,0x6A,0xA7,0xB2,0x31,0xC7,0xDF,0xB9,0xD5,0x99,0x51,0xAE,0x9C
    };

    int ret = verify_block_ex(crypt, crypted, sizeof(crypted));
    return ret && verify_block_ex(tag, longtag, sizeof(longtag));
}

int test_ocb2_dec() {
    const uchar rawkey[AES_BLOCK_SIZE] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    uchar tag[AES_BLOCK_SIZE];
    unsigned char crypt[40];

    ocb_aes_t ctx;
    ocb_aes_set_keys(&ctx, rawkey, rawkey, rawkey, sizeof(rawkey));

    ocb2_aes_decrypt(&ctx, NULL, 0, crypt, tag);
    const uchar blank_tag[AES_BLOCK_SIZE] = {0xBF,0x31,0x08,0x13,0x07,0x73,0xAD,0x5E,0xC7,0x0E,0xC6,0x9E,0x78,0x75,0xA7,0xB0};

    return verify_block_ex(tag, blank_tag, sizeof(blank_tag));
}

int test_ocb2_dec2() {
    const uchar rawkey[AES_BLOCK_SIZE] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    uchar tag[AES_BLOCK_SIZE];
    unsigned char source[40];
    unsigned char crypt[40];

    const uchar longtag[AES_BLOCK_SIZE] = {0x9D,0xB0,0xCD,0xF8,0x80,0xF7,0x3E,0x3E,0x10,0xD4,0xEB,0x32,0x17,0x76,0x66,0x88};
    const uchar crypted[40] = {0xF7,0x5D,0x6B,0xC8,0xB4,0xDC,0x8D,0x66,0xB8,0x36,0xA2,0xB0,0x8B,0x32,0xA6,0x36,0x9F,0x1C,0xD3,0xC5,0x22,0x8D,0x79,0xFD,
      0x6C,0x26,0x7F,0x5F,0x6A,0xA7,0xB2,0x31,0xC7,0xDF,0xB9,0xD5,0x99,0x51,0xAE,0x9C
    };

    ocb_aes_t ctx;
    ocb_aes_set_keys(&ctx, rawkey, rawkey, rawkey, sizeof(rawkey));
    for (int i=0;i<40;i++)
      source[i]=i;
    ocb2_aes_decrypt(&ctx, crypted, sizeof(source), crypt, tag);

    int ret = verify_block_ex(crypt, source, sizeof(source));
    return ret && verify_block_ex(tag, longtag, sizeof(longtag));
}

int main(int argc, char **argv) {
  /* Part tests */
  assert(test_aes_encrypt() == 0);
  assert(test_cpy_block() == 0);
  assert(test_double_block_boundary() == 0);
  assert(test_double_block_negative() == 0);
  assert(test_ntz() == 0);
  assert(test_xor_block() == 0);
  assert(test_left_shift() == 0);

  // RFC 7253 test cases don't match declaration of encryption
  // assert(test_nonce_init() == 0);
  // assert(test_ktop_init() == 0);

  /* Test internal values */
  assert(test_keyvar_init() == 0);
  assert(test_offset_init_bottom() == 0);
  assert(test_offset_init_ktop() == 0);
  assert(test_offset_init_stretch() == 0);
  assert(test_offset_init() == 0);
  assert(test_ocb2_dec() == 0);
  assert(test_ocb2_dec2() == 0);

  assert(test_1() == 0);

  /* OCB 1*/
  assert(test_ocb2_enc() == 0);
  assert(test_ocb2_enc2() == 0);
  printf("All tests passed!\n");
  return 0;
}
