# UME: Unstructured Mesh Explorations

"Ume" is the romanization of the Japanese word for "plum" (梅, or うめ).
Sort of pronounced in English as if you were asking the question
"oo-meh?"

## Description

UME is a collection of data structures and algorithms for parallel
unstructured meshes.  It is intended to be used to explore the
co-design space of hardware and software to support computational
fluid dynamics simulation packages.

## CMake Configuration Options

Package-specific CMake options include:
* `-DUSE_CATCH2=NO` will disable Ctest testing that relies on version 3 
  of the Catch2 testing framework.
* `-DUSE_KOKKOS=YES` will enable Kokkos support, and requires Kokkos
  to be found on the CMake module path.
* `-DUSE_MPI=YES` will enable MPI support, and requires MPI compilers
  to be available on the search path.
* `-DUSE_OPENACC=YES` will enable OpenACC support, and requires the
  C++ compiler to support it.



