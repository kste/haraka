#include "haraka.h"
#include <stdio.h>
#include <stdlib.h>

void test_implementations() {
  unsigned char *in = (unsigned char *)calloc(64*8, sizeof(unsigned char));
  unsigned char *out256 = (unsigned char *)calloc(32*8, sizeof(unsigned char));
  unsigned char *out512 = (unsigned char *)calloc(32*8, sizeof(unsigned char));
  unsigned char testvector256[32] = {0x80, 0x27, 0xcc, 0xb8, 0x79, 0x49, 0x77, 0x4b,
                                     0x78, 0xd0, 0x54, 0x5f, 0xb7, 0x2b, 0xf7, 0x0c,
                                     0x69, 0x5c, 0x2a, 0x09, 0x23, 0xcb, 0xd4, 0x7b,
                                     0xba, 0x11, 0x59, 0xef, 0xbf, 0x2b, 0x2c, 0x1c};

 unsigned char testvector512[32] = {0xbe, 0x7f, 0x72, 0x3b, 0x4e, 0x80, 0xa9, 0x98,
                                    0x13, 0xb2, 0x92, 0x28, 0x7f, 0x30, 0x6f, 0x62,
                                    0x5a, 0x6d, 0x57, 0x33, 0x1c, 0xae, 0x5f, 0x34,
                                    0xdd, 0x92, 0x77, 0xb0, 0x94, 0x5b, 0xe2, 0xaa};



  int i;

  // Input for testvector
  for(i = 0; i < 256; i++) {
    in[i] = i % 32;
  }

  haraka256_8x(out256, in);

  // Verify output
  for(i = 0; i < 32; i++) {
    if (out256[i % 32] != testvector256[i]) {
      printf("Error: testvector incorrect.\n");
      return;
    }
  }

  free(in);
  free(out256);
  free(out512);
}

void print_uint8(u128 v, const unsigned char *name) {
    int i;
    printf("%s = ", name);
    for(i = 0; i < 16; i++) {
        printf("%x", ((unsigned char *)&v)[i]);
    }
    printf("\n");
}


void haraka256(unsigned char *out, const unsigned char *in); //{
//   u128 s[2], tmp;
//
//   s[0] = LOAD(in);
//   s[1] = LOAD(in + 16);
//
//   AES2(s[0], s[1], 0);
//   MIX2(s[0], s[1]);
//
//   AES2(s[0], s[1], 4);
//   MIX2(s[0], s[1]);
//
//   AES2(s[0], s[1], 8);
//   MIX2(s[0], s[1]);
//
//   AES2(s[0], s[1], 12);
//   MIX2(s[0], s[1]);
//
//   AES2(s[0], s[1], 16);
//   s[0] = XOR(s[0], rc256[20]);
//   s[1] = XOR(s[1], rc256[21]);
//
//   MIX2(s[0], s[1]);
//
//   s[0] = XOR(s[0], LOAD(in));
//   s[1] = XOR(s[1], LOAD(in + 16));
//
//   STORE(out, s[0]);
//   STORE(out + 16, s[1]);
// }

void haraka256_4x(unsigned char *out, const unsigned char *in) {
  u128 s[4][2], tmp;

  s[0][0] = LOAD(in);
  s[0][1] = LOAD(in + 16);
  s[1][0] = LOAD(in + 32);
  s[1][1] = LOAD(in + 48);
  s[2][0] = LOAD(in + 64);
  s[2][1] = LOAD(in + 80);
  s[3][0] = LOAD(in + 96);
  s[3][1] = LOAD(in + 112);

  // Round 1
  AES2_4x(s[0], s[1], s[2], s[3], 0);

  MIX2(s[0][0], s[0][1]);
  MIX2(s[1][0], s[1][1]);
  MIX2(s[2][0], s[2][1]);
  MIX2(s[3][0], s[3][1]);

  // Round 2
  AES2_4x(s[0], s[1], s[2], s[3], 4);

  MIX2(s[0][0], s[0][1]);
  MIX2(s[1][0], s[1][1]);
  MIX2(s[2][0], s[2][1]);
  MIX2(s[3][0], s[3][1]);

  // Round 3
  AES2_4x(s[0], s[1], s[2], s[3], 8);

  MIX2(s[0][0], s[0][1]);
  MIX2(s[1][0], s[1][1]);
  MIX2(s[2][0], s[2][1]);
  MIX2(s[3][0], s[3][1]);

  // Round 4
  AES2_4x(s[0], s[1], s[2], s[3], 12);

  MIX2(s[0][0], s[0][1]);
  MIX2(s[1][0], s[1][1]);
  MIX2(s[2][0], s[2][1]);
  MIX2(s[3][0], s[3][1]);

  // Round 5
  AES2_4x(s[0], s[1], s[2], s[3], 16);
  s[0][0] = XOR(s[0][0], rc256[20]);
  s[0][1] = XOR(s[0][1], rc256[21]);
  s[1][0] = XOR(s[1][0], rc256[20]);
  s[1][1] = XOR(s[1][1], rc256[21]);
  s[2][0] = XOR(s[2][0], rc256[20]);
  s[2][1] = XOR(s[2][1], rc256[21]);
  s[3][0] = XOR(s[3][0], rc256[20]);
  s[3][1] = XOR(s[3][1], rc256[21]);

  MIX2(s[0][0], s[0][1]);
  MIX2(s[1][0], s[1][1]);
  MIX2(s[2][0], s[2][1]);
  MIX2(s[3][0], s[3][1]);

  // Feed Forward
  s[0][0] = XOR(s[0][0], LOAD(in));
  s[0][1] = XOR(s[0][1], LOAD(in + 16));
  s[1][0] = XOR(s[1][0], LOAD(in + 32));
  s[1][1] = XOR(s[1][1], LOAD(in + 48));
  s[2][0] = XOR(s[2][0], LOAD(in + 64));
  s[2][1] = XOR(s[2][1], LOAD(in + 80));
  s[3][0] = XOR(s[3][0], LOAD(in + 96));
  s[3][1] = XOR(s[3][1], LOAD(in + 112));

  STORE(out, s[0][0]);
  STORE(out + 16, s[0][1]);
  STORE(out + 32, s[1][0]);
  STORE(out + 48, s[1][1]);
  STORE(out + 64, s[2][0]);
  STORE(out + 80, s[2][1]);
  STORE(out + 96, s[3][0]);
  STORE(out + 112, s[3][1]);
}

void haraka256_8x(unsigned char *out, const unsigned char *in) {
  // This is faster on Skylake, the code below is faster on Haswell.
  haraka256_4x(out, in);
  haraka256_4x(out + 128, in + 128);
  return;
}

void haraka512(unsigned char *out, const unsigned char *in) {
  u128 s[4], tmp;

  s[0] = LOAD(in);
  s[1] = LOAD(in + 16);
  s[2] = LOAD(in + 32);
  s[3] = LOAD(in + 48);

  AES4(s[0], s[1], s[2], s[3], 0);
  MIX4(s[0], s[1], s[2], s[3]);

  AES4(s[0], s[1], s[2], s[3], 8);
  MIX4(s[0], s[1], s[2], s[3]);

  AES4(s[0], s[1], s[2], s[3], 16);
  MIX4(s[0], s[1], s[2], s[3]);

  AES4(s[0], s[1], s[2], s[3], 24);
  MIX4(s[0], s[1], s[2], s[3]);

  AES4(s[0], s[1], s[2], s[3], 32);
  MIX4(s[0], s[1], s[2], s[3]);

  s[0] = XOR(s[0], LOAD(in));
  s[1] = XOR(s[1], LOAD(in + 16));
  s[2] = XOR(s[2], LOAD(in + 32));
  s[3] = XOR(s[3], LOAD(in + 48));

  TRUNCSTORE(out, s[0], s[1], s[2], s[3]);
}

void haraka512_4x(unsigned char *out, const unsigned char *in) {
  u128 s[4][4], tmp;

  s[0][0] = LOAD(in);
  s[0][1] = LOAD(in + 16);
  s[0][2] = LOAD(in + 32);
  s[0][3] = LOAD(in + 48);
  s[1][0] = LOAD(in + 64);
  s[1][1] = LOAD(in + 80);
  s[1][2] = LOAD(in + 96);
  s[1][3] = LOAD(in + 112);
  s[2][0] = LOAD(in + 128);
  s[2][1] = LOAD(in + 144);
  s[2][2] = LOAD(in + 160);
  s[2][3] = LOAD(in + 176);
  s[3][0] = LOAD(in + 192);
  s[3][1] = LOAD(in + 208);
  s[3][2] = LOAD(in + 224);
  s[3][3] = LOAD(in + 240);

  AES4_4x(s[0], s[1], s[2], s[3], 0);
  MIX4(s[0][0], s[0][1], s[0][2], s[0][3]);
  MIX4(s[1][0], s[1][1], s[1][2], s[1][3]);
  MIX4(s[2][0], s[2][1], s[2][2], s[2][3]);
  MIX4(s[3][0], s[3][1], s[3][2], s[3][3]);

  AES4_4x(s[0], s[1], s[2], s[3], 8);
  MIX4(s[0][0], s[0][1], s[0][2], s[0][3]);
  MIX4(s[1][0], s[1][1], s[1][2], s[1][3]);
  MIX4(s[2][0], s[2][1], s[2][2], s[2][3]);
  MIX4(s[3][0], s[3][1], s[3][2], s[3][3]);

  AES4_4x(s[0], s[1], s[2], s[3], 16);
  MIX4(s[0][0], s[0][1], s[0][2], s[0][3]);
  MIX4(s[1][0], s[1][1], s[1][2], s[1][3]);
  MIX4(s[2][0], s[2][1], s[2][2], s[2][3]);
  MIX4(s[3][0], s[3][1], s[3][2], s[3][3]);

  AES4_4x(s[0], s[1], s[2], s[3], 24);
  MIX4(s[0][0], s[0][1], s[0][2], s[0][3]);
  MIX4(s[1][0], s[1][1], s[1][2], s[1][3]);
  MIX4(s[2][0], s[2][1], s[2][2], s[2][3]);
  MIX4(s[3][0], s[3][1], s[3][2], s[3][3]);

  AES4_4x(s[0], s[1], s[2], s[3], 32);
  MIX4(s[0][0], s[0][1], s[0][2], s[0][3]);
  MIX4(s[1][0], s[1][1], s[1][2], s[1][3]);
  MIX4(s[2][0], s[2][1], s[2][2], s[2][3]);
  MIX4(s[3][0], s[3][1], s[3][2], s[3][3]);


  s[0][0] = XOR(s[0][0], LOAD(in));
  s[0][1] = XOR(s[0][1], LOAD(in + 16));
  s[0][2] = XOR(s[0][2], LOAD(in + 32));
  s[0][3] = XOR(s[0][3], LOAD(in + 48));
  s[1][0] = XOR(s[1][0], LOAD(in + 64));
  s[1][1] = XOR(s[1][1], LOAD(in + 80));
  s[1][2] = XOR(s[1][2], LOAD(in + 96));
  s[1][3] = XOR(s[1][3], LOAD(in + 112));
  s[2][0] = XOR(s[2][0], LOAD(in + 128));
  s[2][1] = XOR(s[2][1], LOAD(in + 144));
  s[2][2] = XOR(s[2][2], LOAD(in + 160));
  s[2][3] = XOR(s[2][3], LOAD(in + 176));
  s[3][0] = XOR(s[3][0], LOAD(in + 192));
  s[3][1] = XOR(s[3][1], LOAD(in + 208));
  s[3][2] = XOR(s[3][2], LOAD(in + 224));
  s[3][3] = XOR(s[3][3], LOAD(in + 240));

  TRUNCSTORE(out, s[0][0], s[0][1], s[0][2], s[0][3]);
  TRUNCSTORE(out + 32, s[1][0], s[1][1], s[1][2], s[1][3]);
  TRUNCSTORE(out + 64, s[2][0], s[2][1], s[2][2], s[2][3]);
  TRUNCSTORE(out + 96, s[3][0], s[3][1], s[3][2], s[3][3]);
}

void haraka512_8x(unsigned char *out, const unsigned char *in) {
  // This is faster on Skylake, the code below is faster on Haswell.
  haraka512_4x(out, in);
  haraka512_4x(out + 128, in + 256);
}
