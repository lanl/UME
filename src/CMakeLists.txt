# Ume/src/CMakeLists.txt

add_subdirectory(Ume)

add_executable(ume_serial ume_serial.cc)
target_link_libraries(ume_serial Ume)
set_target_properties(ume_serial PROPERTIES
  COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
  COMPILE_OPTIONS "${WARNING_FLAGS}"
  )

add_executable(txt2bin txt2bin.cc)
target_link_libraries(txt2bin Ume)
target_compile_options(txt2bin PRIVATE ${WARNING_FLAGS}
  ${COMMON_COMPILE_OPTIONS})


#############################################################################
# Things that depend on MPI
#############################################################################
if(USE_MPI)

  add_executable(ume_mpi ume_mpi.cc)

  if(USE_CALIPER)
    message(STATUS "Building Ume MPI with Caliper Profiling")
    target_include_directories(ume_mpi PUBLIC ${caliper_INCLUDE_DIR})
    target_compile_definitions(ume_mpi PUBLIC USE_CALI)
    target_link_libraries(ume_mpi Ume caliper)
  else()
    target_link_libraries(ume_mpi Ume)
    set_target_properties(ume_mpi PROPERTIES
      COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
      COMPILE_OPTIONS "${WARNING_FLAGS}"
      )
  endif()
endif()


#############################################################################
# Things that depend on Kokkos
#############################################################################

if(USE_KOKKOS)
  add_executable(kokkos_hello kokkos_hello.cc)
  target_link_libraries(kokkos_hello
    Ume
    Kokkos::kokkos
    )
endif()


#############################################################################
# Things that depend on OpenACC
#############################################################################

if(USE_OPENACC)
  add_executable(openacc_hello openacc_hello.cc)
  target_link_libraries(openacc_hello
    Ume
    OpenACC::OpenACC_CXX
    )
endif()
