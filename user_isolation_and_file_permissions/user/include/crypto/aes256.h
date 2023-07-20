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
#ifndef INCLUDE_AES256_h_
#define INCLUDE_AES256_h_

#include "types.h"
#include "crypto.h"

typedef struct {
  uchar nonce[4];
  uchar iv[8];
  uchar ctr[4];
} rfc3686_blk;

typedef struct {
  uchar key[32];
  uchar enckey[32];
  rfc3686_blk blk;
} aes256_context;

void aes256_init(aes256_context *ctx, uchar *key);
void aes256_done(aes256_context *ctx);
void aes256_encrypt_ecb(aes256_context *ctx, uchar *buf);
void aes256_setCtrBlk(aes256_context *ctx, rfc3686_blk *blk);
void aes256_encrypt_ctr(aes256_context *ctx, uchar *buf, uint sz);

#endif // INCLUDE_AES256_h_
