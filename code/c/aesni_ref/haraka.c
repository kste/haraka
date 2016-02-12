#include "wmmintrin.h"
#include "emmintrin.h"
#include "smmintrin.h"
#include "loop.h"
#include "helpers.h"
#include <stdio.h>

#define ROUNDS (5)
#define AES_PER_ROUND (2)

int haraka512256(unsigned char *hash, const unsigned char *msg) {
	// stuff we need
	int i, j;
	__m128i s[4], tmp, rcon;
	__m128i MSB64 = _mm_set_epi32(0xFFFFFFFF,0xFFFFFFFF,0,0);

	// set initial round constant
	rcon = _mm_set_epi32(1,1,1,1);

	// initialize state to msg
	s[0] = _mm_load_si128(&((__m128i*)msg)[0]);
	s[1] = _mm_load_si128(&((__m128i*)msg)[1]);
	s[2] = _mm_load_si128(&((__m128i*)msg)[2]);
	s[3] = _mm_load_si128(&((__m128i*)msg)[3]);

	printf("= input state =\n");
	printstate512(s[0], s[1], s[2], s[3]);

	for (i = 0; i < ROUNDS; ++i) {
		// aes round(s)
		for (j = 0; j < AES_PER_ROUND; ++j) {
			s[0] = _mm_aesenc_si128(s[0], rcon);
			s[1] = _mm_aesenc_si128(s[1], rcon);
			s[2] = _mm_aesenc_si128(s[2], rcon);
			s[3] = _mm_aesenc_si128(s[3], rcon);
			rcon = _mm_slli_epi32(rcon, 1);
		}

		printf("= round %d : after aes layer =\n", i);
		printstate512(s[0], s[1], s[2], s[3]);
		
		// mixing
		tmp  = _mm_unpacklo_epi32(s[0], s[1]);
		s[0] = _mm_unpackhi_epi32(s[0], s[1]);
		s[1] = _mm_unpacklo_epi32(s[2], s[3]);
		s[2] = _mm_unpackhi_epi32(s[2], s[3]);
		s[3] = _mm_unpacklo_epi32(s[0], s[2]);
		s[0] = _mm_unpackhi_epi32(s[0], s[2]);
		s[2] = _mm_unpackhi_epi32(s[1],  tmp);
		s[1] = _mm_unpacklo_epi32(s[1],  tmp);

		printf("= round %d : after mix layer =\n", i);
		printstate512(s[0], s[1], s[2], s[3]);

		// little-endian mixing (not used)
		// tmp  = _mm_unpackhi_epi32(s[1], s[0]);
		// s[0] = _mm_unpacklo_epi32(s[1], s[0]);
		// s[1] = _mm_unpackhi_epi32(s[3], s[2]);
		// s[2] = _mm_unpacklo_epi32(s[3], s[2]);
		// s[3] = _mm_unpackhi_epi32(s[2], s[0]);
		// s[0] = _mm_unpacklo_epi32(s[2], s[0]);
		// s[2] = _mm_unpacklo_epi32(tmp,  s[1]);
		// s[1] = _mm_unpackhi_epi32(tmp,  s[1]);
	}

	printf("= output from permutation =\n");
	printstate512(s[0], s[1], s[2], s[3]);

	// xor message to get DM effect
	s[0] = _mm_xor_si128(s[0], _mm_load_si128(&((__m128i*)msg)[0]));
	s[1] = _mm_xor_si128(s[1], _mm_load_si128(&((__m128i*)msg)[1]));
	s[2] = _mm_xor_si128(s[2], _mm_load_si128(&((__m128i*)msg)[2]));
	s[3] = _mm_xor_si128(s[3], _mm_load_si128(&((__m128i*)msg)[3]));

	printf("= after feed-forward =\n");
	printstate512(s[0], s[1], s[2], s[3]);

	// truncate and store result
	_mm_maskmoveu_si128(s[0], MSB64, (hash-8));
	_mm_maskmoveu_si128(s[1], MSB64, (hash+0));
	_mm_storel_epi64((__m128i*)(hash + 16), s[2]);
	_mm_storel_epi64((__m128i*)(hash + 24), s[3]);
}

int haraka256256(unsigned char *hash, const unsigned char *msg) {
	// stuff we need
	int i, j;
	__m128i s[2], tmp, rcon;
	__m128i MSB64 = _mm_set_epi32(0xFFFFFFFF,0xFFFFFFFF,0,0);

	// set initial round constant
	rcon = _mm_set_epi32(1,1,1,1);

	// initialize state to msg
	s[0] = _mm_load_si128(&((__m128i*)msg)[0]);
	s[1] = _mm_load_si128(&((__m128i*)msg)[1]);

	printf("= input state =\n");
	printstate256(s[0], s[1]);

	for (i = 0; i < ROUNDS; ++i) {
		// aes round(s)
		for (j = 0; j < AES_PER_ROUND; ++j) {
			s[0] = _mm_aesenc_si128(s[0], rcon);
			s[1] = _mm_aesenc_si128(s[1], rcon);
			rcon = _mm_slli_epi32(rcon, 1);
		}

		printf("= round %d : after aes layer =\n", i);
		printstate256(s[0], s[1]);
		
		// mixing
		tmp = _mm_unpacklo_epi32(s[0], s[1]);
		s[1] = _mm_unpackhi_epi32(s[0], s[1]);
		s[0] = tmp;

		printf("= round %d : after mix layer =\n", i);
		printstate256(s[0], s[1]);
	}

	printf("= output from permutation =\n");
	printstate256(s[0], s[1]);

	// xor message to get DM effect
	s[0] = _mm_xor_si128(s[0], _mm_load_si128(&((__m128i*)msg)[0]));
	s[1] = _mm_xor_si128(s[1], _mm_load_si128(&((__m128i*)msg)[1]));

	printf("= after feed-forward =\n");
	printstate256(s[0], s[1]);

	// store result
	_mm_storeu_si128((__m128i*)hash, s[0]);
	_mm_storeu_si128((__m128i*)(hash + 16), s[1]);
}

int main() {
	// allocate memory for input and digest
	unsigned char *msg = (unsigned char *)calloc(64, sizeof(unsigned char));
	unsigned char *digest = (unsigned char *)calloc(32, sizeof(unsigned char));
	int i;

	// set some input bytes
	for (i = 0; i < 64; ++i)
		msg[i] = i;

	// print input
	printf("= input bytes =\n");
	printbytes(msg, 64); printf("\n");

	// run Haraka-512/256
	haraka512256(digest, msg);

	// print output
	printf("= haraka-512/256 output bytes =\n"); 
	printbytes(digest, 32); printf("\n");

	// run Haraka-256/256
	haraka256256(digest, msg);

	// print output
	printf("= haraka-256/256 output bytes =\n"); 
	printbytes(digest, 32); printf("\n");

	return 0;	
}