# Haraka - Analysis

This folder contains python scripts to construct the mixed integer linear
programming (MILP) model used in the security analysis of Haraka.

## Examples
Count the number of active S-boxes for AES-like designs. Parameters like the number of rounds or state dimensions
can be specified in the *.yaml file.
```
python3 aesmilp.py --sbox --config examples/aeslike.yaml
```

Finding the optimal truncated differential attack for Haraka.
```
python3 aesmilp.py --truncated --config examples/haraka.yaml
```

For more details on this see our paper.
