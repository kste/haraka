#ifndef PERMUTE_H
#define PERMUTE_H

void chacha_permute(unsigned char output[64],const unsigned char input [64]);

void load_rc();
void haraka512256(unsigned char out[32], const unsigned char in[64]);
void haraka256256(unsigned char out[32], const unsigned char in[32]);
void haraka512256_8x(unsigned char out[32*8], const unsigned char in[64*8]);
void haraka256256_8x(unsigned char out[32*8], const unsigned char in[32*8]);

#endif
