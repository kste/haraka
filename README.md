# Haraka v2

Haraka v2 is a secure and efficient short-input (256 or 512 bits) hash function, designed 
to be very fast on modern platforms which support AES-NI. One of the main applications 
for such a design is the use in hash-based signature schemes like XMSS and SPHINCS.
For more information see our [paper](https://eprint.iacr.org/2016/098).

This repository provides various implementations in [code/](https://github.com/kste/haraka/tree/master/code).
In [code/c/aesni_optimized](https://github.com/kste/haraka/tree/master/code/c/aesni_optimized), one can find
an implementation processing 4 or 8 blocks in parallel.


## Performance

The performance is measured in cycles per byte (cpb) processed. The following numbers
correspond to Intel Skylake using the [optimized implementation](https://github.com/kste/haraka/tree/master/code/c/aesni_optimized).

Variant | 1x | 4x | 8x
------- | ------- | ------- | -------
Haraka256 | 0.72 cpb | 0.63 cpb  | 0.63 cpb 
Haraka512 | 1.02 cpb  | 0.72 cpb  | 0.72 cpb 

## SPHINCS
[SPHINCS](https://sphincs.cr.yp.to/) is a post-quantum secure hash-based digital signature scheme. The performance
of SPHINCS strongly correlates with the performance of the underlying hash function and can be significantly
improved by using an optimized construction.

A SPHINCS implementation instantiated with Haraka can be found in [supercop/crypto_sign/](https://github.com/kste/haraka/tree/master/supercop/crypto_sign/sphincs256haraka/aesni), which can also be used for benchmarks with 
[Supercop](https://bench.cr.yp.to/supercop.html).

This optimized implementation has the following perfomance figures on Intel Skylake:

Operation | Cycles
------------ | -------------
KeyGeneration | 1.340.338
Signing | 20.782.894
Verify | 415.586


## Reference

Haraka v2 - Efficient Short-Input Hashing for Post-Quantum Applications

Stefan Kölbl and Martin M. Lauridsen and Florian Mendel and Christian Rechberger
https://eprint.iacr.org/2016/098
