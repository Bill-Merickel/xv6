#ifndef INCLUDE_CRYPTO_h_
#define INCLUDE_CRYPTO_h_

#include "types.h"

#define AES256_KEY_SIZE_BYTES (32)
#define SHA256_SIZE_BYTES (32)

/**
 * Hash an arbitrary number of bytes using the SHA256 algorithm.
 * See user/src/crypto/test/test_sha256.c for an example use case.
 * 
 * @param data Data to hash. Byte array (unsigned or signed) of size len bytes.
 * @param len The length of the data to hash
 * @param hash Resulting hash stored in array of size SHA256_SIZE_BYTES
 */
void sha256(const void *data, uint len, uchar *hash);

/**
 * Encrypt an arbitrary number of bytes using the AES256-CTR algorithm.
 * See user/src/crypto/test/test_aes256.c for an example use case.
 * 
 * @param key Symmetric key. Byte array of size AES256_KEY_SIZE_BYTES.
 * @param buf Data to encrypt. Byte array (either char or uchar) of size len.
 * @param len The length of the data to encrypt
 */
void aes256_encrypt(uchar *key, void *buf, uint len);

/**
 * Decrypt an arbitrary number of bytes using the AES256-CTR algorithm.
 * See user/src/crypto/test/test_aes256.c for an example use case.
 * 
 * Alias to aes256_encrypt since symmetric encryption does not require different
 * encryption / decryption steps.
 * 
 * @param key Symmetric key. Byte array of size AES256_KEY_SIZE_BYTES.
 * @param buf Data to encrypt. Byte array (either char or uchar) of size len.
 * @param len The length of the data to encrypt
 */
static inline void aes256_decrypt(uchar *key, void *buf, uint len) {
  aes256_encrypt(key, buf, len);
}

#endif // INCLUDE_CRYPTO_h_
