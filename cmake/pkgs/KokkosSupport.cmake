
option(USE_KOKKOS "Enable support for Kokkos code")

if(USE_KOKKOS)
  find_package(Kokkos REQUIRED)
endif()
