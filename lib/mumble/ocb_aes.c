#include "ocb_aes.h"

#include <openssl/aes.h>
#include <string.h>

typedef unsigned char uchar;

static void aes_encrypt(const char *key, const char *in, char *out);
static void aes_decrypt(const char *key, const char *in, char *out);
static void cpy_block(char *dest, const char *source);
static void double_block(const uchar *in, uchar *out);
static void ocb2_decrypt_block(ocb_aes_t *ctx, const uchar *in, uchar *out, uchar *delta, uchar *checksum);
static void ocb3_encrypt_block(ocb_aes_t *ctx, const char *block, const char *keyvar, char *out, char *offset, char *checksum);
static void ocb3_encrypt_block_final(ocb_aes_t *ctx, int i, int size, const char *block, char *out, char *offset, char *checksum, const char *plain, int plainl, char *tag);
static void ocb3_hash_plaintext(ocb_aes_t *ctx, const char *plaintext, int plaintextl, char *sum);
static void keyvar_init(const char *key, char *keyvar);
static void left_shift(const uchar *in, int inl, uchar *out, int bits);
static int ntz(int i);
static void ocb2_encrypt_block(ocb_aes_t *ctx, const uchar *in, uchar *out, uchar *delta, uchar *checksum);
static void ocb2_encrypt_block_final(ocb_aes_t *ctx, const uchar *in, const uchar inl, uchar *out, uchar *delta, uchar *checksum, uchar *tag);
static void offset_init(const ocb_aes_t *aes, char *offset);
static void triple_block(const uchar *in, uchar *out);
static void xor_block(const char *a, const char *b, char *out);
static void xor_block_ex(const char *a, const char *b, char *out, int block_size);
static char zero_block[AES_BLOCK_SIZE] = {0};

void ocb_aes_set_keys(ocb_aes_t *ctx, const char *key, const char *enc_iv, const char *dec_iv, int ivlen) {
  memcpy(ctx->key, key, AES_KEY_SIZE);
  memcpy(ctx->enc_iv, enc_iv, ivlen);
  memcpy(ctx->dec_iv, dec_iv, ivlen);

  ctx->ivlen = ivlen;
  ctx->taglen = 96;
}

/* **************** *
 * PUBLIC FUNCTIONS *
 * **************** *
 */
int ocb3_aes_encrypt(ocb_aes_t *ctx, const char *in, int inl, const char *plain, int plainl, char *out, char *tag) {
  char checksum[AES_BLOCK_SIZE] = {0};
  char keyvar[AES_BLOCK_SIZE] = {0};
  char offset[AES_BLOCK_SIZE] = {0};

  keyvar_init(ctx->key, keyvar);
  offset_init(ctx, offset);

  int log2 = ntz(0);
  int i;
  for (i = 0; i < inl / AES_BLOCK_SIZE; i++) {
    uchar *pout = out + (i * AES_BLOCK_SIZE);
    const uchar *pin = in + (i * AES_BLOCK_SIZE);

    // See if we need to bump the offset
    int log2_next = ntz(i);
    if (log2_next > log2) {
      char double_temp[AES_BLOCK_SIZE];
      double_block(keyvar, double_temp);
      cpy_block(keyvar, double_temp);
      log2 = log2_next;
    }

    // Encrypt this block
    ocb3_encrypt_block(ctx, pin, keyvar, pout, offset, checksum);
  }

  ocb3_encrypt_block_final(ctx, i, inl - (i * AES_BLOCK_SIZE), in, out, offset, checksum, plain, plainl, tag);

  return 0;
}

int ocb3_aes_decrypt(ocb_aes_t *ctx, const char *in, int inl, char *out) {
}


int ocb2_aes_encrypt(ocb_aes_t *ctx, const uchar *in, int inl, uchar *out, uchar *tag) {
  uchar checksum[AES_BLOCK_SIZE] = {0};
  uchar delta[AES_BLOCK_SIZE];
  aes_encrypt(ctx->key, ctx->enc_iv, delta);

  int i;
  for (i = 0; i < inl / AES_BLOCK_SIZE; i++) {
    char *pout = out + (i * AES_BLOCK_SIZE);
    const char *pin = in + (i * AES_BLOCK_SIZE);

    ocb2_encrypt_block(ctx, pin, pout, delta, checksum);
  }

  i *= AES_BLOCK_SIZE;
  ocb2_encrypt_block_final(ctx, in + i, inl - i, out + i, delta, checksum, tag);

  return 0;
}

int ocb2_aes_decrypt(ocb_aes_t *ctx, const uchar *in, int inl, uchar *out, uchar *tag) {
  uchar checksum[AES_BLOCK_SIZE] = {0};
  uchar delta[AES_BLOCK_SIZE];
  aes_encrypt(ctx->key, ctx->dec_iv, delta);

  const uchar *pin = in;
  uchar *pout = out;
  int i;
  for (i = 0; i < inl / AES_BLOCK_SIZE; i++) {
    ocb2_decrypt_block(ctx, pin, pout, delta, checksum);
    pin += AES_BLOCK_SIZE;
    pout += AES_BLOCK_SIZE;
  }
  ocb2_encrypt_block_final(ctx, pin, inl - (i*AES_BLOCK_SIZE), pout, delta, checksum, tag);
}

/* ************************* *
 * OCB3 ENCRYPTING FUNCTIONS *
 * ************************* *
 */
static void ocb3_encrypt_block(ocb_aes_t *ctx, const char *block, const char *keyvar, char *out, char *offset, char *checksum) {
  char checksum_next[AES_BLOCK_SIZE];
  char cipher_in[AES_BLOCK_SIZE];
  char cipher[AES_BLOCK_SIZE];
  char offset_next[AES_BLOCK_SIZE];

  xor_block(offset, keyvar, offset_next);
  xor_block(block, offset_next, cipher_in);
  aes_encrypt(ctx->key, cipher_in, cipher);
  xor_block(offset_next, cipher, out);
  xor_block(checksum, block, checksum_next);

  cpy_block(checksum, checksum_next);
  cpy_block(offset, offset_next);
}

static void ocb3_compute_tag(ocb_aes_t *ctx, const char *checksum, const char *offset, const char *plain, int plainl, char *tag) {
  char keyvar0[AES_BLOCK_SIZE];
  char keyvar[AES_BLOCK_SIZE];
  aes_encrypt(ctx->key, zero_block, keyvar0);
  double_block(keyvar0, keyvar);

  char chksum_xor_offset[AES_BLOCK_SIZE];
  xor_block(checksum, offset, chksum_xor_offset);

  char chksum_xor_offset_xor_keyvar[AES_BLOCK_SIZE];
  xor_block(chksum_xor_offset, keyvar, chksum_xor_offset_xor_keyvar);

  char enciphered[AES_BLOCK_SIZE];
  aes_encrypt(ctx->key, chksum_xor_offset_xor_keyvar, enciphered);

  char hash[AES_BLOCK_SIZE];
  ocb3_hash_plaintext(ctx, plain, plainl, hash);

  xor_block(hash, enciphered, tag);
}

static void ocb3_encrypt_block_final(ocb_aes_t *ctx, int i, int size, const char *block, char *out, char *offset, char *checksum, const char *plain, int plainl, char *tag) {
  char keyvar[AES_BLOCK_SIZE];
  char checksum_next[AES_BLOCK_SIZE];

  const char *block_current = block + (i * AES_BLOCK_SIZE);
  char *cipher_out = out + (i * AES_BLOCK_SIZE);

  if (size > 0) {
    aes_encrypt(ctx->key, zero_block, keyvar);
    char offset_next[AES_BLOCK_SIZE];
    xor_block(keyvar, offset, offset_next);

    char pad[AES_BLOCK_SIZE];
    aes_encrypt(ctx->key, offset_next, pad);

    xor_block_ex(pad, block_current, cipher_out, size);

    memset(pad, 0, AES_BLOCK_SIZE);
    memcpy(pad, block_current, size);
    pad[size] = 0x80;
    xor_block(checksum, pad, checksum_next);

    ocb3_compute_tag(ctx, checksum_next, offset_next, plain, plainl, tag);
  } else {
    ocb3_compute_tag(ctx, checksum, offset, plain, plainl, tag);
  }
}

/* ***************** *
 * HASHING FUNCTIONS *
 * ***************** *
 */
static void ocb3_hash_block(ocb_aes_t *ctx, const char *plaintext, char *sum, char *offset, char *keyvar);
static void ocb3_hash_block_final(ocb_aes_t *ctx, const char *plaintext, int size, char *sum, char *offset, char *keyvar);

static void ocb3_hash_plaintext(ocb_aes_t *ctx, const char *plaintext, int plaintextl, char *sum) {
  char keyvar[AES_BLOCK_SIZE];
  char offset[AES_BLOCK_SIZE] = {0};

  keyvar_init(ctx->key, keyvar);
  memset(sum, 0, AES_BLOCK_SIZE);

  int i;
  int log2 = ntz(0);
  for (i = 0; i < (plaintextl - (plaintextl % AES_BLOCK_SIZE)); i += AES_BLOCK_SIZE) {
    int log2_next = ntz(i);
    if (ntz(i) > log2) {
      // double it
      char double_temp[AES_BLOCK_SIZE];
      double_block(keyvar, double_temp);
      cpy_block(keyvar, double_temp);
      log2 = log2_next;
    }

    ocb3_hash_block(ctx, plaintext + i, sum, offset, keyvar);
  }

  if (i < plaintextl) {
    ocb3_hash_block_final(ctx, plaintext, plaintextl - i, sum, offset, keyvar);
  }
}

static void ocb3_hash_block(ocb_aes_t *ctx, const char *plaintext, char *sum, char *offset, char *keyvar) {
  char offset_next[AES_BLOCK_SIZE];
  xor_block(offset, keyvar, offset_next);

  char xored_plaintext[AES_BLOCK_SIZE];
  xor_block(plaintext, offset_next, xored_plaintext);

  char enciphered[AES_BLOCK_SIZE];
  aes_encrypt(ctx->key, xored_plaintext, enciphered);

  xor_block(enciphered, sum, sum);
  cpy_block(offset, offset_next);
}

static void ocb3_hash_block_final(ocb_aes_t *ctx, const char *plaintext, int size, char *sum, char *offset, char *keyvar) {
  aes_encrypt(ctx->key, zero_block, keyvar);
  char padded[AES_BLOCK_SIZE] = {0};
  memcpy(padded, plaintext, size);
  padded[size] = 0x80;

  ocb3_hash_block(ctx, padded, sum, offset, keyvar);
}

/* ********************** *
 * OCB2 ENCRYPT FUNCTIONS *
 * ********************** *
 */

static void ocb2_encrypt_block(ocb_aes_t *ctx, const uchar *in, uchar *out, uchar *delta, uchar *checksum) {
  uchar checksum_new[AES_BLOCK_SIZE];
  uchar delta_new[AES_BLOCK_SIZE];
  uchar encrypted[AES_BLOCK_SIZE];
  uchar xor_in[AES_BLOCK_SIZE];

  double_block(delta, delta_new);

  xor_block(delta_new, in, xor_in);

  aes_encrypt(ctx->key, xor_in, encrypted);
  xor_block(encrypted, delta_new, out);

  xor_block(checksum, in, checksum_new);

  cpy_block(checksum, checksum_new);
  cpy_block(delta, delta_new);
}

static void ocb2_encrypt_block_final(ocb_aes_t *ctx, const uchar *in, const uchar inl, uchar *out, uchar *delta, uchar *checksum, uchar *tag) {
  uchar checksum_new[AES_BLOCK_SIZE];
  uchar delta_new[AES_BLOCK_SIZE];
  uchar delta_tag[AES_BLOCK_SIZE];
  uchar pad[AES_BLOCK_SIZE*2];
  uchar pad_xor_padded[AES_BLOCK_SIZE];
  uchar size_xor_delta[AES_BLOCK_SIZE];
  uchar tag_in[AES_BLOCK_SIZE];
  uchar zero_and_size[AES_BLOCK_SIZE] = {0};

  // S2
  double_block(delta, delta_new);

  // size_xor_delta = bitlen(in) xor delta
  zero_and_size[AES_BLOCK_SIZE - 1] = inl * 8;
  xor_block(zero_and_size, delta_new, size_xor_delta);

  // Pad = ENCIPHER(size_xor_delta)
  aes_encrypt(ctx->key, size_xor_delta, pad);

  // C_m = M_m xor Pad[1..b]
  xor_block_ex(in, pad, out, inl);

  // Tmp = M_m || Pad[b+1..BLOCKLEN]
  memcpy(pad, in, inl);

  // Checksum = Checksum xor Tmp
  xor_block(pad, checksum, checksum_new);

  // Offset = times3(Offset)
  triple_block(delta_new, delta_tag);

  // T = ENCIPHER(K, Checksum xor Offset)
  xor_block(delta_tag, checksum_new, tag_in);
  aes_encrypt(ctx->key, tag_in, tag);

  cpy_block(checksum, checksum_new);
}

/* ********************** *
 * OCB2 DECRYPT FUNCTIONS *
 * ********************** *
 */

static void ocb2_decrypt_block(ocb_aes_t *ctx, const uchar *in, uchar *out, uchar *delta, uchar *checksum) {
  uchar checksum_new[AES_BLOCK_SIZE];
  uchar delta_new[AES_BLOCK_SIZE];
  uchar delta_xor_in[AES_BLOCK_SIZE];
  uchar deciphered[AES_BLOCK_SIZE];

  // Delta = times2(Delta)
  double_block(delta, delta_new);

  // M_i = Delta xor DECIPHER(C_i xor Delta)
  xor_block(delta_new, in, delta_xor_in);
  aes_decrypt(ctx->key, delta_xor_in, deciphered);
  xor_block(deciphered, delta_new, out);

  // Checksum = Checksum xor M_i
  xor_block(checksum, out, checksum_new);

  cpy_block(checksum, checksum_new);
  cpy_block(delta, delta_new);
}

/* ************** *
 * OCB3 FUNCTIONS *
 * ************** *
 */
static void nonce_init(const ocb_aes_t *ctx, uchar *nonce) {
  memset(nonce, 0, AES_BLOCK_SIZE);
  // RFC7253 states the first 7 bits of the nonce is the taglen mod 128.
  // Test cases in the RFC, however, want the first 7 bits to be zero.
  // nonce[0] = ((ctx->taglen % (AES_BLOCK_SIZE*8)) << 1);

  // set prefix to iv
  // TODO: Do not depend on fixed iv length
  int iv_prefix_index = (AES_BLOCK_SIZE - AES_IV_SIZE) - 1;
  nonce[iv_prefix_index] |= 0x01;

  for (int i = 0; i < AES_IV_SIZE; i++) {
    int index = (AES_BLOCK_SIZE - AES_IV_SIZE) + i;
    nonce[index] = ctx->enc_iv[i];
  }
}

static void ktop_init(const ocb_aes_t *ctx, const char *nonce, char *ktop) {
  uchar temp[AES_BLOCK_SIZE];
  cpy_block(temp, nonce);

  temp[AES_BLOCK_SIZE-1] &= 0xC0;
  aes_encrypt(ctx->key, temp, ktop);
}

static void stretch_init(const char *ktop, char *stretch) {
  char xor_out[AES_BLOCK_SIZE/2];
  cpy_block(stretch, ktop);
  xor_block_ex(ktop, ktop + 1, xor_out, AES_BLOCK_SIZE/2);
  memcpy(stretch + AES_BLOCK_SIZE, xor_out, AES_BLOCK_SIZE/2);
}

static void offset_init(const ocb_aes_t *ctx, char *offset) {
  const int stretch_size = AES_BLOCK_SIZE + (AES_BLOCK_SIZE/2);
  uchar nonce[AES_BLOCK_SIZE];
  int bottom;
  uchar ktop[AES_BLOCK_SIZE];
  uchar stretch[stretch_size];
  uchar shifted_block[stretch_size];

  memset(offset, 0, AES_BLOCK_SIZE);
  nonce_init(ctx, nonce);
  bottom = 0;
  bottom = (nonce[AES_BLOCK_SIZE - 1] & 0x1F);
  ktop_init(ctx, nonce, ktop);
  stretch_init(ktop, stretch);
  left_shift(stretch, stretch_size, shifted_block, bottom);
  cpy_block(offset, shifted_block);
}

static void keyvar_init(const char *key, char *keyvar) {
  aes_encrypt(key, zero_block, keyvar);

  char temp_block[AES_KEY_SIZE];
  double_block(keyvar, temp_block);
  double_block(temp_block, keyvar);
}

/* ***************** *
 * UTILITY FUNCTIONS *
 * ***************** *
 */
static void aes_encrypt(const char *key, const char *in, char *out) {
  AES_KEY aes_key;
  AES_set_encrypt_key(key, AES_KEY_SIZE*8, &aes_key);
  AES_encrypt(in, out, &aes_key);
}

static void aes_decrypt(const char *key, const char *in, char *out) {
  AES_KEY aes_key;
  AES_set_decrypt_key(key, AES_KEY_SIZE*8, &aes_key);
  AES_decrypt(in, out, &aes_key);
}

static void cpy_block(char *dest, const char *source) {
  memcpy(dest, source, AES_BLOCK_SIZE);
}

static void double_block(const uchar *in, uchar *out) {
  left_shift(in, AES_BLOCK_SIZE, out, 1);

  if (*in & 0x80) {
    out[AES_BLOCK_SIZE-1] ^= 0x87;
  }
}

static void left_shift(const uchar *in, int inl, uchar *out, int bits) {
  for (int i = 0; i < inl; i++) {
    /* Get the index of bytes to shift. bits can be greater than 8, requiring an index jump.
     * 1. divide bits by 8 to get the number of bytes to jump.
     * 2. Shift that byte by bits % 8 and copy to destination.
     * 3. Go to the next byte, and mask the highest (bits % 8) bits for backfill.
     * 4. Shift the highest bits right by 8 - (bits % 8) to put them in the low order spot.
     * 5. OR into destination.
     */
    int index = i + (bits / 8);
    if (index < inl) {
      // If the byte to shift from exists, then do a shift
      out[i] = in[i + (bits / 8)] << (bits % 8);

      if (index + 1 < inl) {
        // If the next byte exists, backfill the low order bits
        // Byte to backfill in from
        uchar next_byte = in[i + (bits / 8) + 1];
        uchar next_bits = next_byte >> (8 - (bits % 8));

        // Assign backfill bits
        out[i] |= next_bits;
      }
    } else {
      // If the byte to shift from doesn't exist, then fill with zeros
      out[i] = 0x00;
    }
  }
}

// Find log base 2 of i
static int ntz(int i) {
  const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
  const unsigned int S[] = {1, 2, 4, 8, 16};

  unsigned int out = 0;
  for (int x = 4; x >= 0; x--) {
    if (i & b[x]) {
      i >>= S[x];
      out |= S[x];
    }
  }

  return out;
}

static void triple_block(const uchar *in, uchar *out) {
  uchar temp[AES_BLOCK_SIZE];
  double_block(in, temp);
  xor_block(in, temp, out);
}

static void xor_block(const char *a, const char *b, char *out) {
  xor_block_ex(a, b, out, AES_BLOCK_SIZE);
}

static void xor_block_ex(const char *a, const char *b, char *out, int block_size) {
  for (int i = 0; i < block_size; i++) {
    out[i] = a[i] ^ b[i];
  }
}
