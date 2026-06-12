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
