/*
Timing code for optimized implementation of Haraka.
*/

#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#define __USE_GNU
#include <sched.h>



#include "haraka.h"
#include "timing.h"

typedef void (*hash_function)(unsigned char*, const unsigned char*);

// Measures how many cycles func requires to process a random input.
double timeit(hash_function func, int inlen, int outlen) {
  unsigned char *in, *out;
  unsigned long long timer = 0;
  double timings[NUM_TIMINGS];

  int i, j;

  srand(0);
  in = malloc(inlen);
  out = malloc(outlen);

  for (i = -100; i < NUM_TIMINGS; i++) {
    //Get random input
    for (j = 0; j < inlen; j++) {
      in[j] = rand() & 0xff;
    }

    timer = startTimer();
    for(j = 0; j < ITERATIONS; j++) {
        func(out, in);
    }
    timer = endTimer() - timer;

    if (i >= 0 && i < NUM_TIMINGS) {
      timings[i] = ((double)timer) / inlen / ITERATIONS;
    }
  }

  //Get Median
  qsort(timings, NUM_TIMINGS, sizeof(double), compareDouble);

  free(out);
  free(in);
  return timings[NUM_TIMINGS / 2];
}

int main() {
  test_implementations();
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset); CPU_SET(7, &cpuset);

  if (sched_setaffinity(getpid(), sizeof cpuset, &cpuset) != 0) perror("setaffinity");

  printf("Haraka-256 1x: %f cycles per byte\n", timeit(haraka256, 32, 32));
  printf("Haraka-256 4x: %f cycles per byte\n", timeit(haraka256_4x, 4*32, 4*32));
  printf("Haraka-256 8x: %f cycles per byte\n", timeit(haraka256_8x, 8*32, 8*32));
  //
  printf("Haraka-512 1x: %f cycles per byte\n", timeit(haraka512, 64, 32));
  printf("Haraka-512 4x: %f cycles per byte\n", timeit(haraka512_4x, 4*64, 4*32));
  printf("Haraka-512 8x: %f cycles per byte\n", timeit(haraka512_8x, 8*64, 8*32));
}
