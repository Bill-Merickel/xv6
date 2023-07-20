//
//  A byte-oriented AES-256-CTR implementation.
//  Based on the code available at http://www.literatecode.com/aes256
//  Complies with RFC3686, http://tools.ietf.org/html/rfc3686
//
//  Copyright (c) 2013 Ilya O. Levin, Literatecode
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//
//  GitHub Repository: ilvn/aes256ctr
//
#include "types.h"
#include "user.h"
#include "crypto.h"

static void assert_equal(const void *actual, const void *expected, uint len,
                         const char *err);

static uchar key[AES256_KEY_SIZE_BYTES] = {
  0xFF, 0x7A, 0x61, 0x7C, 0xE6, 0x91, 0x48, 0xE4,
  0xF1, 0x72, 0x6E, 0x2F, 0x43, 0x58, 0x1D, 0xE2,
  0xAA, 0x62, 0xD9, 0xF8, 0x05, 0x53, 0x2E, 0xDF,
  0xF1, 0xEE, 0xD6, 0x87, 0xFB, 0x54, 0x15, 0x3D
};

static const uchar expected_buf[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  0x20, 0x21, 0x22, 0x23
};

static const uchar expected_encrypted_buf[] = {
  0xEB, 0x6C, 0x52, 0x82, 0x1D, 0x0B, 0xBB, 0xF7,
  0xCE, 0x75, 0x94, 0x46, 0x2A, 0xCA, 0x4F, 0xAA,
  0xB4, 0x07, 0xDF, 0x86, 0x65, 0x69, 0xFD, 0x07,
  0xF4, 0x8C, 0xC0, 0xB5, 0x83, 0xD6, 0x07, 0x1F,
  0x1E, 0xC0, 0xE6, 0xB8
};

int main(int argc, char *argv[]) {
  uchar buf[sizeof(expected_buf)];

  printf(1, "test_aes256: begin\n");

  for (uint i = 0; i < sizeof(buf); i++) {
    buf[i] = expected_buf[i];
  }

  assert_equal(buf, expected_buf, sizeof(buf), "initial plaintext");

  // encrypt data in buf using key
  aes256_encrypt(key, buf, sizeof(buf));
  assert_equal(buf, expected_encrypted_buf, sizeof(buf), "encrypted buf");

  // decrypt data in buf using key
  aes256_decrypt(key, buf, sizeof(buf));
  assert_equal(buf, expected_buf, sizeof(buf), "decrypted buf");
  
  printf(1, "test_aes256: pass\n");
  exit();
}

static void assert_equal(const void *actual, const void *expected, uint len,
                         const char *err) {
  const uchar *actual_bytes = (const uchar *) actual;
  const uchar *expected_bytes = (const uchar *) expected;

  for (uint i = 0; i < len; i++) {
    if (actual_bytes[i] != expected_bytes[i]) {
      printf(2, "test_aes256: fail on %s! mismatch byte %d, expected 0x%x but "
                "got 0x%x\n", err, i, expected_bytes[i], actual_bytes[i]);
      exit();
    }
  }
}
