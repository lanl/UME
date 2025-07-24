# UME: Unstructured Mesh Explorations

[![Contributor
Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](code_of_conduct.md)


## Description

UME is a collection of data structures and algorithms for parallel
unstructured meshes. It is intended to be used to explore the
co-design space of hardware and software to support computational
fluid dynamics simulation packages.

## Installation

UME is a [CMake](https://cmake.org)-based package written in
C++20. The basic steps for building UME are:
  1. Install external packages; 
  2. Do a cmake configuration step; 
  3. Build the libraries and executables.

The primary product of UME is a library and header files, as well as
some example driver executables.

### External Packages

UME relies on several external packages. These can be installed easily
using the [spack](https://github.com/spack/spack) package
manager. The spack specs for all packages are:
```yaml
spack:
  specs:
    - catch2
    - cmake
    - doxygen
    - git
    - graphviz
    - kokkos+openmp std=20
    - openmpi
  packages:
    catch2:
      require: '@3:'
```

### CMake Configuration Options

CMake options can be specified either with the interactive `ccmake`
tool, or on the command line of `cmake`, preceded by the `-D`
flag. Package-specific CMake options for UME include:
* `USE_CATCH2=NO` will disable Ctest testing that relies on version 3 
  of the Catch2 testing framework.
* `USE_KOKKOS=YES` will enable Kokkos support, and requires Kokkos
  to be found in the environment (UME does not currently use Kokkos).
* `USE_MPI=YES` will enable MPI support, and requires MPI compilers
  to be available on the search path.
* `USE_OPENACC=YES` will enable OpenACC support, and requires the
  C++ compiler to support it (UME does not currently use OpenACC).


### Generated Executables

After a successful build, two example driver executables will be
generated that are of interest to the general community: `ume_serial`
and `ume_mpi`.  Each exercises the gradient operators found in the
`Ume/gradient.hh` headers.  The MPI version `ume_mpi` is the most
realistic. The usage is:
```shell
% ume_serial <filename>
```
Where `<filename>` is the complete file name for an UME binary input
file (distributed separately). Or

```shell
% mpirun -np <n> ume_mpi <prefix>
```
Where there is expected to be a set of files named as if generated
with `sprintf(filename, '%s.%05d.ume', prefix, rank)` for 0 <= rank <
n. 

There will also be two utility executables: `txt2bin` and `scale_mesh`.
The `txt2bin` utility takes in an `.umetxt` file and converts it to
a binary representation that is ingestible by Ume.
```shell
% txt2bin <infile> <filename>
```
Where `<infile>` is the complete file name for an UME text input
file and `<filename>` is the name of the UME binary file to be 
used by UME.

The `scale_mesh` utility takes in an UME binary input file and 
increases the size of the mesh by a user-chosed factor. The scaling
factor is currently constrained to be a factor of 2.
```shell
% scale_mesh <prefix> 8
```
Where `<filename>` is the base file name for a set of UME binary input
files.

You can also run `scale_mesh` with MPI
```
% mpirun -np <n> scale_mesh <prefix> 8
```
### Run gradient kernel multiple times

By default, all the kernels is run once. There is an option to run these kernels in `ume_mpi` multiple times to increase the computations for benchmarking purposes using command line argument. 

```shell
 % mpirun -np <n> ume_mpi <prefix> -i <number of iterations>
 ```

## Project Name

"Ume" is also the romanization of the Japanese word for "plum" (梅, or
うめ).  Sort of pronounced in English as if you were asking the
question "oo-meh?"


## Authors

UME was created by Paul Henning (<phenning@lanl.gov>).

UME is currently maintained by:
Austin Bullock (<abullock@lanl.gov>)
Sumathi Lakshmiranganatha (<sumathil@lanl.gov>)
Jered Dominguez-Trujillo (<jereddt@lanl.gov>)

## Release

UME is released as open source under a BSD-3 License.  For more
details, please see the [LICENSE.md](LICENSE.md) and
[NOTICE.md](NOTICE.md) files.

`LANL C23016`

&copy; 2023. Triad National Security, LLC. All rights reserved.
