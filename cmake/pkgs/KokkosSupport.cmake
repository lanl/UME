
set(USE_KOKKOS False CACHE BOOL "Enable support for Kokkos code")

if(USE_KOKKOS)
  find_package(Kokkos REQUIRED)
  set(KOKKOS_LINK_LIBRARIES
    Kokkos::kokkos
    )
endif()
