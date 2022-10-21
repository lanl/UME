# Ume/src/CMakeLists.txt

if(USE_KOKKOS)
  add_executable(kokkos_hello kokkos_hello.cc)
  target_link_libraries(kokkos_hello ${KOKKOS_LINK_LIBRARIES})
endif()

if(USE_OPENACC)
  add_executable(openacc_hello openacc_hello.cc)
  target_link_libraries(openacc_hello ${OPENACC_LINK_LIBRARIES})
endif()
