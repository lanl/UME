# UME: Unstructured Mesh Explorations

"Ume" is the romanization of the Japanese word for "plum" (梅, or うめ).
Sort of pronounced in English as if you were asking the question
"oo-meh?"

## Description

UME is a collection of data structures and algorithms for parallel
unstructured meshes. It is intended to be used to explore the
co-design space of hardware and software to support computational
fluid dynamics simulation packages.

## Installation

UME is a (CMake)[https://cmake.org]-based package written in
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
  to be found in the environment.
* `USE_MPI=YES` will enable MPI support, and requires MPI compilers
  to be available on the search path.
* `USE_OPENACC=YES` will enable OpenACC support, and requires the
  C++ compiler to support it.



