#ifndef HELPERS_H
#define HELPERS_H
#include <stdint.h>

/////////////
// HELPERS //
/////////////
void print_block(__m128i);
void printbytes(unsigned char *, int);
void printstate512(__m128i a, __m128i b, __m128i c, __m128i d);
void printstate256(__m128i a, __m128i b);

#endif
