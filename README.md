# Haraka

Haraka is a secure and efficient hash function, designed specifically
to process short inputs and be very fast on modern platforms which
support AES-NI. One of the main applications for such a design is the 
use in hash-based signature schemes like XMSS and SPHINCS.

## Features
- Supports AES-NI
- Low Latency
- High performance (below 1 cycle/byte on Skylake)

This repository provides a reference implementation and parts of the
software used for the security analysis. For more information see
our paper.

## Reference

Haraka - Efficient Short-Input Hashing for Post-Quantum Applications

Stefan Kölbl and Martin M. Lauridsen and Florian Mendel and Christian Rechberger
https://eprint.iacr.org/2016/098
