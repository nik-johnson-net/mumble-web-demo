#ifndef OCB_AES_H_
#define OCB_AES_H_

#include <stdint.h>

#define AES_BLOCK_SIZE 16
#define AES_IV_SIZE 12
#define AES_KEY_SIZE 16

typedef struct {
  char key[AES_KEY_SIZE];
  char enc_iv[AES_KEY_SIZE];
  char dec_iv[AES_KEY_SIZE];
  uint8_t ivlen;
  uint8_t taglen;
} ocb_aes_t;

void ocb_aes_set_keys(ocb_aes_t *ctx, const char *key, const char *enc_iv, const char *dec_iv, int ivlen);
int ocb2_aes_encrypt(ocb_aes_t *ctx, const unsigned char *in, int inl, unsigned char *out, unsigned char *tag);
int ocb2_aes_decrypt(ocb_aes_t *ctx, const unsigned char *in, int inl, unsigned char *out, unsigned char *tag);
int ocb3_aes_encrypt(ocb_aes_t *ctx, const char *in, int inl, const char *plain, int plainl, char *out, char *tag);
int ocb3_aes_decrypt(ocb_aes_t *ctx, const char *in, int inl, char *out);

#endif
