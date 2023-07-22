//
//  SHA-256 implementation, Mark 2
//
//  Copyright (c) 2010,2014 Literatecode, http://www.literatecode.com
//  Copyright (c) 2022 Ilia Levin (ilia@levin.sg)
//
//  Permission to use, copy, modify, and distribute this software for any
//  purpose with or without fee is hereby granted, provided that the above
//  copyright notice and this permission notice appear in all copies.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
//  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
//  GitHub Repository: ilvn/SHA256
//
#include "types.h"
#include "user.h"
#include "crypto.h"

void print_hex_char(uchar num) {
  char *digits = "0123456789abcdef";
  char hex_num[] = {
    digits[(num >> 4) & 0xf],
    digits[num & 0xf],
    '\0'
  };
  printf(1, "%s", (char *) hex_num);
}

int main(int argc, char *argv[]) {
  printf(1, "test_sha256: begin\n");
  char *buf[] = {
    "",
    "e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855",

    "abc",
    "ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad",

    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    "248d6a61 d20638b8 e5c02693 0c3e6039 a33ce459 64ff2167 f6ecedd4 19db06c1",

    "The quick brown fox jumps over the lazy dog",
    "d7a8fbb3 07d78094 69ca9abc b0082e4f 8d5651e4 6d3cdb76 2d02d0bf 37c9e592",

    "The quick brown fox jumps over the lazy cog", // avalanche effect test
    "e4c4d8f3 bf76b692 de791a17 3e053211 50f7a345 b46484fe 427f6acc 7ecc81be",

    "bhn5bjmoniertqea40wro2upyflkydsibsk8ylkmgbvwi420t44cq034eou1szc1k0mk46oeb7ktzmlxqkbte2sy",
    "9085df2f 02e0cc45 5928d0f5 1b27b4bf 1d9cd260 a66ed1fd a11b0a3f f5756d99"
  };

  const uint tests_total = sizeof(buf) / sizeof(buf[0]);
  uchar hash[SHA256_SIZE_BYTES];

  if (0 != (tests_total % 2)) {
    printf(2, "test_sha256: invalid tests\n");
    return 1;
  }

  for (uint i = 0; i < tests_total; i += 2) {
    // hash a string from buf using sha256 and store in hash
    sha256(buf[i], strlen(buf[i]), hash);
    printf(1, "input = '%s'\ndigest: %s\nresult: ", buf[i], buf[i + 1]);
    for (uint j = 0; j < SHA256_SIZE_BYTES; j++) {
      print_hex_char(hash[j]);
      printf(1, "%s", ((j % 4) == 3) ? " " : "");
    }
    printf(1, "\n\n");
  }

  printf(1, "test_sha256: end, check the equivalence of hashes\n");
  exit();
  return 0;
}
