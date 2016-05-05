#include <stdint.h>
#include "wmmintrin.h"
#include "emmintrin.h"
#include "smmintrin.h"
#include "loop.h"

#define ROUNDS (5)
#define AES_PER_ROUND (2)

#define CHACHA_ROUNDS 12

#define U32V(x) \
  ((x) & 0xffffffff)

#define ROTL32(x,c) \
  ((((x) << c) | ((x) >> (32-c))) & 0xffffffff)

#define ROTATE(v,c) (ROTL32(v,c))
#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v),1))

#define QUARTERROUND(a,b,c,d) \
  x[a] = PLUS(x[a],x[b]); x[d] = ROTATE(XOR(x[d],x[a]),16); \
  x[c] = PLUS(x[c],x[d]); x[b] = ROTATE(XOR(x[b],x[c]),12); \
  x[a] = PLUS(x[a],x[b]); x[d] = ROTATE(XOR(x[d],x[a]), 8); \
  x[c] = PLUS(x[c],x[d]); x[b] = ROTATE(XOR(x[b],x[c]), 7);


void chacha_permute(unsigned char out[64],const unsigned char in [64])
{
  uint32_t x[16];
  int i;

  for (i = 0;i < 16;i++)
  {
    x[i] = in[4*i+3];
    x[i] <<= 8;
    x[i] |= in[4*i+2];
    x[i] <<= 8;
    x[i] |= in[4*i+1];
    x[i] <<= 8;
    x[i] |= in[4*i+0];
  }

  for (i = CHACHA_ROUNDS;i > 0;i -= 2) 
  {
    QUARTERROUND( 0, 4, 8,12)
    QUARTERROUND( 1, 5, 9,13)
    QUARTERROUND( 2, 6,10,14)
    QUARTERROUND( 3, 7,11,15)
    QUARTERROUND( 0, 5,10,15)
    QUARTERROUND( 1, 6,11,12)
    QUARTERROUND( 2, 7, 8,13)
    QUARTERROUND( 3, 4, 9,14)
  }

//  for (i = 0;i < 16;++i) x[i] = PLUS(x[i],input[i]); // XXX: Bad idea if we later xor the input to the state?
  for (i = 0;i < 16;++i) 
  {
    out[4*i]   =  x[i] & 0xff;
    out[4*i+1] = (x[i] >>  8) & 0xff;
    out[4*i+2] = (x[i] >> 16) & 0xff;
    out[4*i+3] = (x[i] >> 24) & 0xff;
  }
}


void haraka512256(unsigned char out[32], const unsigned char in[64]) {
    // stuff we need
    int i, j;
    __m128i s[4], tmp, rc[40];
    __m128i MSB64 = _mm_set_epi32(0xFFFFFFFF,0xFFFFFFFF,0,0);

    // define round constants
    rc[0] = _mm_set_epi32(0x0684704c,0xe620c00a,0xb2c5fef0,0x75817b9d);
    rc[1] = _mm_set_epi32(0x8b66b4e1,0x88f3a06b,0x640f6ba4,0x2f08f717);
    rc[2] = _mm_set_epi32(0x3402de2d,0x53f28498,0xcf029d60,0x9f029114);
    rc[3] = _mm_set_epi32(0x0ed6eae6,0x2e7b4f08,0xbbf3bcaf,0xfd5b4f79);
    rc[4] = _mm_set_epi32(0xcbcfb0cb,0x4872448b,0x79eecd1c,0xbe397044);
    rc[5] = _mm_set_epi32(0x7eeacdee,0x6e9032b7,0x8d5335ed,0x2b8a057b);
    rc[6] = _mm_set_epi32(0x67c28f43,0x5e2e7cd0,0xe2412761,0xda4fef1b);
    rc[7] = _mm_set_epi32(0x2924d9b0,0xafcacc07,0x675ffde2,0x1fc70b3b);
    rc[8] = _mm_set_epi32(0xab4d63f1,0xe6867fe9,0xecdb8fca,0xb9d465ee);
    rc[9] = _mm_set_epi32(0x1c30bf84,0xd4b7cd64,0x5b2a404f,0xad037e33);
    rc[10] = _mm_set_epi32(0xb2cc0bb9,0x941723bf,0x69028b2e,0x8df69800);
    rc[11] = _mm_set_epi32(0xfa0478a6,0xde6f5572,0x4aaa9ec8,0x5c9d2d8a);
    rc[12] = _mm_set_epi32(0xdfb49f2b,0x6b772a12,0x0efa4f2e,0x29129fd4);
    rc[13] = _mm_set_epi32(0x1ea10344,0xf449a236,0x32d611ae,0xbb6a12ee);
    rc[14] = _mm_set_epi32(0xaf044988,0x4b050084,0x5f9600c9,0x9ca8eca6);
    rc[15] = _mm_set_epi32(0x21025ed8,0x9d199c4f,0x78a2c7e3,0x27e593ec);
    rc[16] = _mm_set_epi32(0xbf3aaaf8,0xa759c9b7,0xb9282ecd,0x82d40173);
    rc[17] = _mm_set_epi32(0x6260700d,0x6186b017,0x37f2efd9,0x10307d6b);
    rc[18] = _mm_set_epi32(0x5aca45c2,0x21300443,0x81c29153,0xf6fc9ac6);
    rc[19] = _mm_set_epi32(0x9223973c,0x226b68bb,0x2caf92e8,0x36d1943a);
    rc[20] = _mm_set_epi32(0xd3bf9238,0x225886eb,0x6cbab958,0xe51071b4);
    rc[21] = _mm_set_epi32(0xdb863ce5,0xaef0c677,0x933dfddd,0x24e1128d);
    rc[22] = _mm_set_epi32(0xbb606268,0xffeba09c,0x83e48de3,0xcb2212b1);
    rc[23] = _mm_set_epi32(0x734bd3dc,0xe2e4d19c,0x2db91a4e,0xc72bf77d);
    rc[24] = _mm_set_epi32(0x43bb47c3,0x61301b43,0x4b1415c4,0x2cb3924e);
    rc[25] = _mm_set_epi32(0xdba775a8,0xe707eff6,0x03b231dd,0x16eb6899);
    rc[26] = _mm_set_epi32(0x6df3614b,0x3c755977,0x8e5e2302,0x7eca472c);
    rc[27] = _mm_set_epi32(0xcda75a17,0xd6de7d77,0x6d1be5b9,0xb88617f9);
    rc[28] = _mm_set_epi32(0xec6b43f0,0x6ba8e9aa,0x9d6c069d,0xa946ee5d);
    rc[29] = _mm_set_epi32(0xcb1e6950,0xf957332b,0xa2531159,0x3bf327c1);
    rc[30] = _mm_set_epi32(0x2cee0c75,0x00da619c,0xe4ed0353,0x600ed0d9);
    rc[31] = _mm_set_epi32(0xf0b1a5a1,0x96e90cab,0x80bbbabc,0x63a4a350);
    rc[32] = _mm_set_epi32(0xae3db102,0x5e962988,0xab0dde30,0x938dca39);
    rc[33] = _mm_set_epi32(0x17bb8f38,0xd554a40b,0x8814f3a8,0x2e75b442);
    rc[34] = _mm_set_epi32(0x34bb8a5b,0x5f427fd7,0xaeb6b779,0x360a16f6);
    rc[35] = _mm_set_epi32(0x26f65241,0xcbe55438,0x43ce5918,0xffbaafde);
    rc[36] = _mm_set_epi32(0x4ce99a54,0xb9f3026a,0xa2ca9cf7,0x839ec978);
    rc[37] = _mm_set_epi32(0xae51a51a,0x1bdff7be,0x40c06e28,0x22901235);
    rc[38] = _mm_set_epi32(0xa0c1613c,0xba7ed22b,0xc173bc0f,0x48a659cf);
    rc[39] = _mm_set_epi32(0x756acc03,0x02288288,0x4ad6bdfd,0xe9c59da1);

    // initialize state to in
    s[0] = _mm_load_si128(&((__m128i*)in)[0]);
    s[1] = _mm_load_si128(&((__m128i*)in)[1]);
    s[2] = _mm_load_si128(&((__m128i*)in)[2]);
    s[3] = _mm_load_si128(&((__m128i*)in)[3]);

    for (i = 0; i < ROUNDS; ++i) {
        // aes round(s)
        for (j = 0; j < AES_PER_ROUND; ++j) {
            s[0] = _mm_aesenc_si128(s[0], rc[4*AES_PER_ROUND*i + 4*j]);
            s[1] = _mm_aesenc_si128(s[1], rc[4*AES_PER_ROUND*i + 4*j + 1]);
            s[2] = _mm_aesenc_si128(s[2], rc[4*AES_PER_ROUND*i + 4*j + 2]);
            s[3] = _mm_aesenc_si128(s[3], rc[4*AES_PER_ROUND*i + 4*j + 3]);
        }

        // mixing
        tmp  = _mm_unpackhi_epi32(s[1], s[0]); \
        s[0] = _mm_unpacklo_epi32(s[1], s[0]); \
        s[1] = _mm_unpackhi_epi32(s[3], s[2]); \
        s[2] = _mm_unpacklo_epi32(s[3], s[2]); \
        s[3] = _mm_unpackhi_epi32(s[2], s[0]); \
        s[0] = _mm_unpacklo_epi32(s[2], s[0]); \
        s[2] = _mm_unpacklo_epi32(tmp,  s[1]); \
        s[1] = _mm_unpackhi_epi32(tmp,  s[1]);        
/*        tmp  = _mm_unpacklo_epi32(s[0], s[1]);
        s[0] = _mm_unpackhi_epi32(s[0], s[1]);
        s[1] = _mm_unpacklo_epi32(s[2], s[3]);
        s[2] = _mm_unpackhi_epi32(s[2], s[3]);
        s[3] = _mm_unpacklo_epi32(s[0], s[2]);
        s[0] = _mm_unpackhi_epi32(s[0], s[2]);
        s[2] = _mm_unpackhi_epi32(s[1],  tmp);
        s[1] = _mm_unpacklo_epi32(s[1],  tmp);*/
    }

    // xor message to get DM effect
    s[0] = _mm_xor_si128(s[0], _mm_load_si128(&((__m128i*)in)[0]));
    s[1] = _mm_xor_si128(s[1], _mm_load_si128(&((__m128i*)in)[1]));
    s[2] = _mm_xor_si128(s[2], _mm_load_si128(&((__m128i*)in)[2]));
    s[3] = _mm_xor_si128(s[3], _mm_load_si128(&((__m128i*)in)[3]));

    // truncate and store result
/*    _mm_maskmoveu_si128(s[0], MSB64, (out-8));
    _mm_maskmoveu_si128(s[1], MSB64, (out+0));
    _mm_storel_epi64((__m128i*)(out + 16), s[2]);
    _mm_storel_epi64((__m128i*)(out + 24), s[3]);*/
    _mm_storel_epi64((__m128i*)(out + 0),  s[0]); \
    _mm_storel_epi64((__m128i*)(out + 8),  s[1]); \
    _mm_storel_epi64((__m128i*)(out + 16), s[2]); \
    _mm_storel_epi64((__m128i*)(out + 24), s[3]);    
}

void haraka256256(unsigned char out[32], const unsigned char in[32]) {
    int i, j;
    __m128i s[2], tmp, rcon, rc[20];
    __m128i MSB64 = _mm_set_epi32(0xFFFFFFFF,0xFFFFFFFF,0,0);

    // define round constants
    rc[0] = _mm_set_epi32(0x0684704c,0xe620c00a,0xb2c5fef0,0x75817b9d);
    rc[1] = _mm_set_epi32(0x8b66b4e1,0x88f3a06b,0x640f6ba4,0x2f08f717);
    rc[2] = _mm_set_epi32(0x3402de2d,0x53f28498,0xcf029d60,0x9f029114);
    rc[3] = _mm_set_epi32(0x0ed6eae6,0x2e7b4f08,0xbbf3bcaf,0xfd5b4f79);
    rc[4] = _mm_set_epi32(0xcbcfb0cb,0x4872448b,0x79eecd1c,0xbe397044);
    rc[5] = _mm_set_epi32(0x7eeacdee,0x6e9032b7,0x8d5335ed,0x2b8a057b);
    rc[6] = _mm_set_epi32(0x67c28f43,0x5e2e7cd0,0xe2412761,0xda4fef1b);
    rc[7] = _mm_set_epi32(0x2924d9b0,0xafcacc07,0x675ffde2,0x1fc70b3b);
    rc[8] = _mm_set_epi32(0xab4d63f1,0xe6867fe9,0xecdb8fca,0xb9d465ee);
    rc[9] = _mm_set_epi32(0x1c30bf84,0xd4b7cd64,0x5b2a404f,0xad037e33);
    rc[10] = _mm_set_epi32(0xb2cc0bb9,0x941723bf,0x69028b2e,0x8df69800);
    rc[11] = _mm_set_epi32(0xfa0478a6,0xde6f5572,0x4aaa9ec8,0x5c9d2d8a);
    rc[12] = _mm_set_epi32(0xdfb49f2b,0x6b772a12,0x0efa4f2e,0x29129fd4);
    rc[13] = _mm_set_epi32(0x1ea10344,0xf449a236,0x32d611ae,0xbb6a12ee);
    rc[14] = _mm_set_epi32(0xaf044988,0x4b050084,0x5f9600c9,0x9ca8eca6);
    rc[15] = _mm_set_epi32(0x21025ed8,0x9d199c4f,0x78a2c7e3,0x27e593ec);
    rc[16] = _mm_set_epi32(0xbf3aaaf8,0xa759c9b7,0xb9282ecd,0x82d40173);
    rc[17] = _mm_set_epi32(0x6260700d,0x6186b017,0x37f2efd9,0x10307d6b);
    rc[18] = _mm_set_epi32(0x5aca45c2,0x21300443,0x81c29153,0xf6fc9ac6);
    rc[19] = _mm_set_epi32(0x9223973c,0x226b68bb,0x2caf92e8,0x36d1943a);

    // initialize state to in
    s[0] = _mm_load_si128(&((__m128i*)in)[0]);
    s[1] = _mm_load_si128(&((__m128i*)in)[1]);

    for (i = 0; i < ROUNDS; ++i) {
        // aes round(s)
        for (j = 0; j < AES_PER_ROUND; ++j) {
            s[0] = _mm_aesenc_si128(s[0], rc[2*AES_PER_ROUND*i + 2*j]);
            s[1] = _mm_aesenc_si128(s[1], rc[2*AES_PER_ROUND*i + 2*j + 1]);
            rcon = _mm_slli_epi32(rcon, 1);
        }
        
        // mixing
        tmp = _mm_unpacklo_epi32(s[0], s[1]);
        s[1] = _mm_unpackhi_epi32(s[0], s[1]);
        s[0] = tmp;
    }

    // xor message to get DM effect
    s[0] = _mm_xor_si128(s[0], _mm_load_si128(&((__m128i*)in)[0]));
    s[1] = _mm_xor_si128(s[1], _mm_load_si128(&((__m128i*)in)[1]));

    // store result
    _mm_storeu_si128((__m128i*)out, s[0]);
    _mm_storeu_si128((__m128i*)(out + 16), s[1]);
}

void haraka256256_8x(unsigned char out[32*8], const unsigned char in[32*8])
{
    int i, j;
    __m128i s[8][2], tmp[8], key, rc[ROUNDS*AES_PER_ROUND*2];

    rc[0] = _mm_set_epi32(0x0684704c,0xe620c00a,0xb2c5fef0,0x75817b9d);
    rc[1] = _mm_set_epi32(0x8b66b4e1,0x88f3a06b,0x640f6ba4,0x2f08f717);
    rc[2] = _mm_set_epi32(0x3402de2d,0x53f28498,0xcf029d60,0x9f029114);
    rc[3] = _mm_set_epi32(0x0ed6eae6,0x2e7b4f08,0xbbf3bcaf,0xfd5b4f79);
    rc[4] = _mm_set_epi32(0xcbcfb0cb,0x4872448b,0x79eecd1c,0xbe397044);
    rc[5] = _mm_set_epi32(0x7eeacdee,0x6e9032b7,0x8d5335ed,0x2b8a057b);
    rc[6] = _mm_set_epi32(0x67c28f43,0x5e2e7cd0,0xe2412761,0xda4fef1b);
    rc[7] = _mm_set_epi32(0x2924d9b0,0xafcacc07,0x675ffde2,0x1fc70b3b);
    rc[8] = _mm_set_epi32(0xab4d63f1,0xe6867fe9,0xecdb8fca,0xb9d465ee);
    rc[9] = _mm_set_epi32(0x1c30bf84,0xd4b7cd64,0x5b2a404f,0xad037e33);
    rc[10] = _mm_set_epi32(0xb2cc0bb9,0x941723bf,0x69028b2e,0x8df69800);
    rc[11] = _mm_set_epi32(0xfa0478a6,0xde6f5572,0x4aaa9ec8,0x5c9d2d8a);
    rc[12] = _mm_set_epi32(0xdfb49f2b,0x6b772a12,0x0efa4f2e,0x29129fd4);
    rc[13] = _mm_set_epi32(0x1ea10344,0xf449a236,0x32d611ae,0xbb6a12ee);
    rc[14] = _mm_set_epi32(0xaf044988,0x4b050084,0x5f9600c9,0x9ca8eca6);
    rc[15] = _mm_set_epi32(0x21025ed8,0x9d199c4f,0x78a2c7e3,0x27e593ec);
    rc[16] = _mm_set_epi32(0xbf3aaaf8,0xa759c9b7,0xb9282ecd,0x82d40173);
    rc[17] = _mm_set_epi32(0x6260700d,0x6186b017,0x37f2efd9,0x10307d6b);
    rc[18] = _mm_set_epi32(0x5aca45c2,0x21300443,0x81c29153,0xf6fc9ac6);
    rc[19] = _mm_set_epi32(0x9223973c,0x226b68bb,0x2caf92e8,0x36d1943a);

    // initialize state to in
    #undef TIMES0
    #define TIMES0(n) \
        s[n][0] = _mm_load_si128(&((__m128i*)(in + n*32))[0]); \
        s[n][1] = _mm_load_si128(&((__m128i*)(in + n*32))[1]);
    REPEAT_THIS(8);

    //Round 1
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[0]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[1]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[2]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[3]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n] = _mm_unpacklo_epi32(s[n][0], s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][0], s[n][1]); \
    s[n][0] = tmp[n];
    REPEAT_THIS(8);

    //Round 2
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[4]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[5]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[6]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[7]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n] = _mm_unpacklo_epi32(s[n][0], s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][0], s[n][1]); \
    s[n][0] = tmp[n];
    REPEAT_THIS(8);

    //Round 3
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[8]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[9]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[10]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[11]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n] = _mm_unpacklo_epi32(s[n][0], s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][0], s[n][1]); \
    s[n][0] = tmp[n];
    REPEAT_THIS(8);

    //Round 4
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[12]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[13]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[14]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[15]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n] = _mm_unpacklo_epi32(s[n][0], s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][0], s[n][1]); \
    s[n][0] = tmp[n];
    REPEAT_THIS(8);

    //Round 5
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[16]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[17]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[18]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[19]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n] = _mm_unpacklo_epi32(s[n][0], s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][0], s[n][1]); \
    s[n][0] = tmp[n];
    REPEAT_THIS(8);
            
    // xor message to get DM effect
    #undef TIMES0
    #define TIMES0(n) \
        s[n][0] = _mm_xor_si128(s[n][0], _mm_load_si128(&((__m128i*)(in + n*32))[0])); \
        s[n][1] = _mm_xor_si128(s[n][1], _mm_load_si128(&((__m128i*)(in + n*32))[1]));
    REPEAT_THIS(8);
    
    #undef TIMES0
    #define TIMES0(n) \
        _mm_storeu_si128((__m128i*)(out + n*32 + 0),  s[n][0]); \
        _mm_storeu_si128((__m128i*)(out + n*32 + 16),  s[n][1]);
    REPEAT_THIS(8);
}

void haraka512256_8x(unsigned char out[32*8], const unsigned char in[64*8]) 
{
    // stuff we need
    int i, j;
    __m128i s[8][4], tmp[8], key, rc[ROUNDS*AES_PER_ROUND*4];
    __m128i MSB64 = _mm_set_epi32(0xFFFFFFFF,0xFFFFFFFF,0,0);

    // initialize state to in
    #undef TIMES0
    #define TIMES0(n) \
        s[n][0] = _mm_load_si128(&((__m128i*)(in + n*64))[0]); \
        s[n][1] = _mm_load_si128(&((__m128i*)(in + n*64))[1]); \
        s[n][2] = _mm_load_si128(&((__m128i*)(in + n*64))[2]); \
        s[n][3] = _mm_load_si128(&((__m128i*)(in + n*64))[3]);
    REPEAT_THIS(8);

    rc[0] = _mm_set_epi32(0x0684704c,0xe620c00a,0xb2c5fef0,0x75817b9d);
    rc[1] = _mm_set_epi32(0x8b66b4e1,0x88f3a06b,0x640f6ba4,0x2f08f717);
    rc[2] = _mm_set_epi32(0x3402de2d,0x53f28498,0xcf029d60,0x9f029114);
    rc[3] = _mm_set_epi32(0x0ed6eae6,0x2e7b4f08,0xbbf3bcaf,0xfd5b4f79);
    rc[4] = _mm_set_epi32(0xcbcfb0cb,0x4872448b,0x79eecd1c,0xbe397044);
    rc[5] = _mm_set_epi32(0x7eeacdee,0x6e9032b7,0x8d5335ed,0x2b8a057b);
    rc[6] = _mm_set_epi32(0x67c28f43,0x5e2e7cd0,0xe2412761,0xda4fef1b);
    rc[7] = _mm_set_epi32(0x2924d9b0,0xafcacc07,0x675ffde2,0x1fc70b3b);
    rc[8] = _mm_set_epi32(0xab4d63f1,0xe6867fe9,0xecdb8fca,0xb9d465ee);
    rc[9] = _mm_set_epi32(0x1c30bf84,0xd4b7cd64,0x5b2a404f,0xad037e33);
    rc[10] = _mm_set_epi32(0xb2cc0bb9,0x941723bf,0x69028b2e,0x8df69800);
    rc[11] = _mm_set_epi32(0xfa0478a6,0xde6f5572,0x4aaa9ec8,0x5c9d2d8a);
    rc[12] = _mm_set_epi32(0xdfb49f2b,0x6b772a12,0x0efa4f2e,0x29129fd4);
    rc[13] = _mm_set_epi32(0x1ea10344,0xf449a236,0x32d611ae,0xbb6a12ee);
    rc[14] = _mm_set_epi32(0xaf044988,0x4b050084,0x5f9600c9,0x9ca8eca6);
    rc[15] = _mm_set_epi32(0x21025ed8,0x9d199c4f,0x78a2c7e3,0x27e593ec);
    rc[16] = _mm_set_epi32(0xbf3aaaf8,0xa759c9b7,0xb9282ecd,0x82d40173);
    rc[17] = _mm_set_epi32(0x6260700d,0x6186b017,0x37f2efd9,0x10307d6b);
    rc[18] = _mm_set_epi32(0x5aca45c2,0x21300443,0x81c29153,0xf6fc9ac6);
    rc[19] = _mm_set_epi32(0x9223973c,0x226b68bb,0x2caf92e8,0x36d1943a);
    rc[20] = _mm_set_epi32(0xd3bf9238,0x225886eb,0x6cbab958,0xe51071b4);
    rc[21] = _mm_set_epi32(0xdb863ce5,0xaef0c677,0x933dfddd,0x24e1128d);
    rc[22] = _mm_set_epi32(0xbb606268,0xffeba09c,0x83e48de3,0xcb2212b1);
    rc[23] = _mm_set_epi32(0x734bd3dc,0xe2e4d19c,0x2db91a4e,0xc72bf77d);
    rc[24] = _mm_set_epi32(0x43bb47c3,0x61301b43,0x4b1415c4,0x2cb3924e);
    rc[25] = _mm_set_epi32(0xdba775a8,0xe707eff6,0x03b231dd,0x16eb6899);
    rc[26] = _mm_set_epi32(0x6df3614b,0x3c755977,0x8e5e2302,0x7eca472c);
    rc[27] = _mm_set_epi32(0xcda75a17,0xd6de7d77,0x6d1be5b9,0xb88617f9);
    rc[28] = _mm_set_epi32(0xec6b43f0,0x6ba8e9aa,0x9d6c069d,0xa946ee5d);
    rc[29] = _mm_set_epi32(0xcb1e6950,0xf957332b,0xa2531159,0x3bf327c1);
    rc[30] = _mm_set_epi32(0x2cee0c75,0x00da619c,0xe4ed0353,0x600ed0d9);
    rc[31] = _mm_set_epi32(0xf0b1a5a1,0x96e90cab,0x80bbbabc,0x63a4a350);
    rc[32] = _mm_set_epi32(0xae3db102,0x5e962988,0xab0dde30,0x938dca39);
    rc[33] = _mm_set_epi32(0x17bb8f38,0xd554a40b,0x8814f3a8,0x2e75b442);
    rc[34] = _mm_set_epi32(0x34bb8a5b,0x5f427fd7,0xaeb6b779,0x360a16f6);
    rc[35] = _mm_set_epi32(0x26f65241,0xcbe55438,0x43ce5918,0xffbaafde);
    rc[36] = _mm_set_epi32(0x4ce99a54,0xb9f3026a,0xa2ca9cf7,0x839ec978);
    rc[37] = _mm_set_epi32(0xae51a51a,0x1bdff7be,0x40c06e28,0x22901235);
    rc[38] = _mm_set_epi32(0xa0c1613c,0xba7ed22b,0xc173bc0f,0x48a659cf);
    rc[39] = _mm_set_epi32(0x756acc03,0x02288288,0x4ad6bdfd,0xe9c59da1);

    //Round 1
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[0]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[1]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[2]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[3]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[4]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[5]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[6]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[7]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n]  = _mm_unpackhi_epi32(s[n][1], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][1], s[n][0]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][3], s[n][2]); \
    s[n][2] = _mm_unpacklo_epi32(s[n][3], s[n][2]); \
    s[n][3] = _mm_unpackhi_epi32(s[n][2], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][2], s[n][0]); \
    s[n][2] = _mm_unpacklo_epi32(tmp[n],  s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(tmp[n],  s[n][1]);
    REPEAT_THIS(8);

    //Round 2
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[8]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[9]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[10]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[11]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[12]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[13]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[14]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[15]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n]  = _mm_unpackhi_epi32(s[n][1], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][1], s[n][0]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][3], s[n][2]); \
    s[n][2] = _mm_unpacklo_epi32(s[n][3], s[n][2]); \
    s[n][3] = _mm_unpackhi_epi32(s[n][2], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][2], s[n][0]); \
    s[n][2] = _mm_unpacklo_epi32(tmp[n],  s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(tmp[n],  s[n][1]);
    REPEAT_THIS(8);

    //Round 3
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[16]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[17]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[18]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[19]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[20]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[21]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[22]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[23]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n]  = _mm_unpackhi_epi32(s[n][1], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][1], s[n][0]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][3], s[n][2]); \
    s[n][2] = _mm_unpacklo_epi32(s[n][3], s[n][2]); \
    s[n][3] = _mm_unpackhi_epi32(s[n][2], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][2], s[n][0]); \
    s[n][2] = _mm_unpacklo_epi32(tmp[n],  s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(tmp[n],  s[n][1]);
    REPEAT_THIS(8);

    //Round 4
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[24]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[25]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[26]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[27]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[28]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[29]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[30]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[31]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n]  = _mm_unpackhi_epi32(s[n][1], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][1], s[n][0]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][3], s[n][2]); \
    s[n][2] = _mm_unpacklo_epi32(s[n][3], s[n][2]); \
    s[n][3] = _mm_unpackhi_epi32(s[n][2], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][2], s[n][0]); \
    s[n][2] = _mm_unpacklo_epi32(tmp[n],  s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(tmp[n],  s[n][1]);
    REPEAT_THIS(8);

    //Round 5
    #undef TIMES0
    #define TIMES0(n) \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[32]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[33]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[34]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[35]); \
    s[n][0] = _mm_aesenc_si128(s[n][0], rc[36]); \
    s[n][1] = _mm_aesenc_si128(s[n][1], rc[37]); \
    s[n][2] = _mm_aesenc_si128(s[n][2], rc[38]); \
    s[n][3] = _mm_aesenc_si128(s[n][3], rc[39]);
    REPEAT_THIS(8);

    #undef TIMES0
    #define TIMES0(n) \
    tmp[n]  = _mm_unpackhi_epi32(s[n][1], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][1], s[n][0]); \
    s[n][1] = _mm_unpackhi_epi32(s[n][3], s[n][2]); \
    s[n][2] = _mm_unpacklo_epi32(s[n][3], s[n][2]); \
    s[n][3] = _mm_unpackhi_epi32(s[n][2], s[n][0]); \
    s[n][0] = _mm_unpacklo_epi32(s[n][2], s[n][0]); \
    s[n][2] = _mm_unpacklo_epi32(tmp[n],  s[n][1]); \
    s[n][1] = _mm_unpackhi_epi32(tmp[n],  s[n][1]);
    REPEAT_THIS(8);
    
    // xor message to get DM effect
    #undef TIMES0
    #define TIMES0(n) \
        s[n][0] = _mm_xor_si128(s[n][0], _mm_load_si128(&((__m128i*)(in + n*64))[0])); \
        s[n][1] = _mm_xor_si128(s[n][1], _mm_load_si128(&((__m128i*)(in + n*64))[1])); \
        s[n][2] = _mm_xor_si128(s[n][2], _mm_load_si128(&((__m128i*)(in + n*64))[2])); \
        s[n][3] = _mm_xor_si128(s[n][3], _mm_load_si128(&((__m128i*)(in + n*64))[3]));
    REPEAT_THIS(8);
    
    #undef TIMES0
    #define TIMES0(n) \
        _mm_storel_epi64((__m128i*)(out + n*32 + 0),  s[n][0]); \
        _mm_storel_epi64((__m128i*)(out + n*32 + 8),  s[n][1]); \
        _mm_storel_epi64((__m128i*)(out + n*32 + 16), s[n][2]); \
        _mm_storel_epi64((__m128i*)(out + n*32 + 24), s[n][3]);
    REPEAT_THIS(8);

/*    _mm_maskmoveu_si128(s[0], MSB64, (out-8));
    _mm_maskmoveu_si128(s[1], MSB64, (out+0));
    _mm_storel_epi64((__m128i*)(out + 16), s[2]);
    _mm_storel_epi64((__m128i*)(out + 24), s[3]);*/
}