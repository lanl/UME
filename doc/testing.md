# UME software stack

The purpose of this document is to record the validated combinations of
the UME software stack. Every listed collection has passed all unit
tests and successfully ran the `ume_mpi` executable with a single-rank
performance mesh of the following form:

```
Mesh Stats
-------------------------------
        Input version: 20250722
        Decomposed Rank 0 (1/1)
        Point dimensions: 3
        Coordinate system: Cartesian
        Iotas dumped: false
        Points: 2146689
        Zones: 2195456 2195456
        Sides: 50724864 50724864
        Edges: 6390144
        Faces: 6340608
        Corners: 17170432 17170432
        Iotas: 0 0
```

The unit tests and `ume_mpi` executable were ran in both debug and
release builds and always with `-DUME_SANITIZE=NO` and `-DUME_SERIAL=NO`
for each enumerated version of Kokkos for each software stack.

## x86_64

```
OS: Red Hat Enterprise Linux 8.10 (Ootpa)
Kernel: 4.18.0-553
ldd (GNU libc) 2.28
CPU: Intel(R) Xeon(R) CPU E5-2660 v3 @ 2.60GHz
GPU: Tesla V100S-PCIE-32GB
Host/MPI compiler: g++ (GCC) 13.2.0
Device compiler: NVIDIA (R) Cuda compiler driver, release 12.9, V12.9.86
CUDA Runtime (cudart): 12.9.79
NVIDIA Linux Driver: 575.57.08
Kokkos: 4.6.2, 4.7.0, 4.7.1, 5.0.0, 5.0.2, 5.1.1
MPI: OpenMPI v4.1.6 (MPI 3.1)
```
Supporting documentation:  
[https://docs.nvidia.com/cuda/archive/12.9.1/cuda-installation-guide-linux/index.html#id60](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-installation-guide-linux/index.html#id60)  
[https://docs.nvidia.com/cuda/archive/12.9.1/cuda-installation-guide-linux/index.html#id61](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-installation-guide-linux/index.html#id61)  
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7)  
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8)

## aarch64

```
OS: Red Hat Enterprise Linux 9.7 (Plow)
Kernel: 5.14.0-611.54.1
ldd (GNU libc) 2.34
CPU: Neoverse-V2
GPU: NVIDIA GH200 480GB
Host/MPI compiler: g++ (GCC) 13.2.0
Device compiler: NVIDIA (R) Cuda compiler driver, 13.2, V13.2.78
CUDA Runtime (cudart): 13.2.75
NVIDIA Linux Driver: 595.71.05
Kokkos: 4.7.1, 5.0.0, 5.0.2, 5.1.1
MPI: OpenMPI v4.1.5 (MPI 3.1)
```
Supporting documentation:  
[https://docs.nvidia.com/cuda/archive/13.2.1/cuda-installation-guide-linux/index.html#id59](https://docs.nvidia.com/cuda/archive/13.2.1/cuda-installation-guide-linux/index.html#id59)  
[https://docs.nvidia.com/cuda/archive/13.2.1/cuda-installation-guide-linux/index.html#id60](https://docs.nvidia.com/cuda/archive/13.2.1/cuda-installation-guide-linux/index.html#id60)  
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7)  
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8)

```
OS: Red Hat Enterprise Linux 9.7 (Plow)
Kernel: 5.14.0-611.54.1
ldd (GNU libc) 2.34
CPU: Neoverse-V2
GPU: NVIDIA GH200 480GB
Host/MPI compiler: g++ (GCC) 13.2.0
Device compiler: NVIDIA (R) Cuda compiler driver, 12.8, V12.8.93
CUDA Runtime (cudart): 12.8.57
NVIDIA Linux Driver: 595.71.05
Kokkos: 4.6.2, 4.7.0, 4.7.1, 5.0.0, 5.0.2, 5.1.1
MPI: OpenMPI v4.1.5 (MPI 3.1)
```
Supporting documentation:  
[https://docs.nvidia.com/cuda/archive/12.8.1/cuda-installation-guide-linux/index.html#id47](https://docs.nvidia.com/cuda/archive/12.8.1/cuda-installation-guide-linux/index.html#id47)  
[https://docs.nvidia.com/cuda/archive/12.8.1/cuda-installation-guide-linux/index.html#id48](https://docs.nvidia.com/cuda/archive/12.8.1/cuda-installation-guide-linux/index.html#id48)  
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7)  
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8)

NOTE: This combination of Cuda runtime and NVIDIA drivers are
incompatible according to the documentation.

```
OS: SLES 15-SP5
Kernel: 5.14.21-150500.55
ldd (GNU libc) 2.31
CPU: Neoverse-V2
GPU: NVIDIA GH200 120GB (x4)
Host/MPI compiler: g++ (GCC) 12.3.0
Device compiler: NVIDIA (R) Cuda compiler driver, 12.6, V12.6.20
CUDA Runtime (cudart): 12.6.37
NVIDIA Linux Driver: 560.35.03
Kokkos: 4.6.2, 4.7.0, 4.7.1, 5.0.0, 5.0.2, 5.1.1
MPI: MPICH v8.1.30 (MPI 3.1)
```
Supporting documentation:  
[https://docs.nvidia.com/cuda/archive/12.6.0/cuda-installation-guide-linux/index.html#id10](https://docs.nvidia.com/cuda/archive/12.6.0/cuda-installation-guide-linux/index.html#id10)
[https://docs.nvidia.com/cuda/archive/12.6.0/cuda-installation-guide-linux/index.html#id11](https://docs.nvidia.com/cuda/archive/12.6.0/cuda-installation-guide-linux/index.html#id11)
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7)  
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8)

```
OS: SLES 15-SP5
Kernel: 5.14.21-150500.55
ldd (GNU libc) 2.31
CPU: Neoverse-V2
GPU: NVIDIA GH200 120GB (x4)
Host/MPI compiler: g++ (GCC) 13.2.0
Device compiler: NVIDIA (R) Cuda compiler driver, 12.6, V12.6.20
CUDA Runtime (cudart): 12.6.37
NVIDIA Linux Driver: 560.35.03
Kokkos: 4.6.2, 4.7.0, 4.7.1, 5.0.0, 5.0.2, 5.1.1
MPI: OpenMPI v5.0.6 (MPI 3.1)
```
Supporting documentation:  
[https://docs.nvidia.com/cuda/archive/12.6.0/cuda-installation-guide-linux/index.html#id10](https://docs.nvidia.com/cuda/archive/12.6.0/cuda-installation-guide-linux/index.html#id10)
[https://docs.nvidia.com/cuda/archive/12.6.0/cuda-installation-guide-linux/index.html#id11](https://docs.nvidia.com/cuda/archive/12.6.0/cuda-installation-guide-linux/index.html#id11)
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id7)  
[https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#id8)

```
OS: SLES 15-SP5
Kernel: 5.14.21-150500.55
ldd (GNU libc) 2.31
CPU: Neoverse-V2
GPU: NVIDIA GH200 120GB (x4)
Host/MPI compiler: clang version 21.1.8
Device compiler: clang version 21.1.8
CUDA Runtime (cudart): 12.6.37
NVIDIA Linux Driver: 560.35.03
Kokkos: 4.6.2, 4.7.0, 4.7.1, 5.0.0, 5.0.2, 5.1.1
MPI: OpenMPI v5.0.6 (MPI 3.1)
```

```
OS: Red Hat Enterprise Linux 9.7 (Plow)
Kernel: 5.14.0-611.54.1
ldd (GNU libc) 2.34
CPU: Neoverse-V2
GPU: NVIDIA GH200 480GB
Host/MPI compiler: clang version 22.1.8
Device compiler: clang version 22.1.8
CUDA Runtime (cudart): 13.2.75
NVIDIA Linux Driver: 595.71.05
Kokkos: 4.7.1, 5.0.0, 5.0.2
MPI: OpenMPI v4.1.5 (MPI 3.1)
```

```
OS: SLES 15-SP5
Kernel: 5.14.21-150500.55
ldd (GNU libc) 2.31
CPU: Neoverse-V2
GPU: NVIDIA GH200 120GB (x4)
Host/MPI compiler: clang version 22.1.8
Device compiler: clang version 22.1.8
CUDA Runtime (cudart): 12.6.37
NVIDIA Linux Driver: 560.35.03
Kokkos: 4.6.2, 4.7.0, 4.7.1, 5.0.0, 5.0.2, 5.1.1
MPI: OpenMPI v5.0.6 (MPI 3.1)
```

## Tests whose failure needs a Debug build

Some regression tests catch the defect they guard against only in a
Debug build. Without the fix, the code has undefined behavior that a
Debug build stops at, but that an optimized build may run through and
still produce the expected output. So a passing Release build (the
`release` cell of `.github/workflows/ci.yml`) says nothing about these
tests; only the Debug builds do.

- A libstdc++ assertion (`_GLIBCXX_ASSERTIONS`, which g++ 15 and later
  define at `-O0`, and which the Debug cells of the CI define for its
  older g++):
  - `comm MPI: exchange with empty remotes and map virtual ranks`
    (MPI builds only): `&buf[offset]` one past the end of the buffer,
    for a remote with no elements.
  - `an empty mesh can still derive its variables`: `&v[0]` on an
    empty vector.
  - `MemoryPoolAllocation releases a claim that is not the last`: an
    index past the end of the list of claims.
  - `entity index ranges are empty on an empty entity`: an `iota_view`
    whose bound is below its start.
  - `txt2bin_truncated_input`: `back()` on an empty string. Without
    the assertion, the code reads the byte before the buffer and
    usually prints the same message, so the test passes without the fix.
- A fill pattern standing in for uninitialized storage. At `-O2` the
  compiler may delete the fill as a dead store:
  - `mesh: default construction sets every header scalar`: a `Mesh`
    constructed with placement new over storage filled with `0xA5`.
  - `std::string binary read from an empty stream`,
    `vector<int> binary read from an empty stream`,
    `Neighbors binary read from a stream that ends after the tag` and
    `vector<Entity::Subset> binary read from an empty stream`:
    `scribble_stack()` writes `0xA5` where the next call's locals will
    be, which is reliable only at `-O0`.
- An `assert`:
  - `error_stop_pool_exhausted`: the memory pool's `Finalize` asserts
    that no claims are outstanding. When `error_stop` finalized before
    printing, a Debug build died at that assertion before printing its
    message; under `NDEBUG` the message still printed. Any `Finalize`
    on the error path, explicit or from a static destructor that
    `std::exit` runs, is the same red: a Debug build dies on `SIGABRT`
    instead of exiting with a failure status, and under `NDEBUG` the
    probe still exits 1.
  - `ume_mpi_interior_zones_in_lower_half` (MPI builds only): the
    search for an interior zone ran off the end of the zone range,
    which a Debug build asserted on.
- A signed overflow:
  - `scale_mesh_bad_scale_-2147483648`: the power-of-2 check
    `scale & (scale - 1)` accepted the most negative `int` only because
    `scale - 1` wraps around, which `-O0` does and an optimizer is not
    required to do.
