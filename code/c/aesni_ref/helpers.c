#include "smmintrin.h"
#include "helpers.h"
#include <stdio.h>

/////////////
// HELPERS //
/////////////
void print_block(__m128i var) {
    uint8_t *val = (uint8_t*) &var;
    //~ printf("%.16llx%.16llx\n", v64val[1], v64val[0]);
    printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			// val[15], val[14], val[13], val[12], val[11], val[10], val[9], val[8], val[7], val[6], val[5], val[4], val[3], val[2], val[1], val[0]);
    		val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15]);
}

void printbytes(unsigned char *m, int len) {
	int i;
	for (i = 0; i < len-1; ++i)
		printf("%02x ", m[i]);
	printf("%02x\n", m[len-1]);
}

void printstate512(__m128i* s) {
	uint8_t *A = (uint8_t*)(&s[0]);
	uint8_t *B = (uint8_t*)(&s[1]);
	uint8_t *C = (uint8_t*)(&s[2]);
	uint8_t *D = (uint8_t*)(&s[3]);

	int i;
	for (i = 0; i < 4; ++i)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			A[i], A[i+4], A[i+8], A[i+12],
			B[i], B[i+4], B[i+8], B[i+12],
			C[i], C[i+4], C[i+8], C[i+12],
			D[i], D[i+4], D[i+8], D[i+12]);
	printf("\n");
}

void printstate256(__m128i* s) {
	uint8_t *A = (uint8_t*)(&s[0]);
	uint8_t *B = (uint8_t*)(&s[1]);

	int i;
	for (i = 0; i < 4; ++i)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
			A[i], A[i+4], A[i+8], A[i+12],
			B[i], B[i+4], B[i+8], B[i+12]);
	printf("\n");
}
