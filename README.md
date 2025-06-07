# Parallel Sequence Alignment

This project was developed as part of the **Concurrent Programming** course at *École Polytechnique*. We aim to parallelize the Needleman-Wunsch sequence alignment algorithm.

## Compilation

To compile all the executables, run:

```bash
make all
```

This will produce four binaries:

* `DW` (Diagonal Wavefront CPU)
* `BW` (Block-Based Column Wavefront CPU)
* `COL` (Sequential)
* `DW_CUDA` (Diagonal Wavefront GPU)

## Usage

* **`./DW` and `./BW`**:
  Runs the parallelized CPU implementations using a fixed input size `N` (that you can modify in the main() function). The runtime will be computed and printed for thread counts ranging from 1 to 8.

* **`./DW_CUDA`**:
  Runs the GPU-accelerated version of the algorithm using 1024 CUDA threads. It benchmarks execution times for input sizes ranging from \$2^7\$ to \$2^{15}\$.
