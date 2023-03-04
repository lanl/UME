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

option(USE_SCORIA "Build Ume with Scoria" ON)
if(USE_SCORIA)
  message(STATUS "Building Ume Serial with Scoria")
  add_executable(ume_serial_client ume_serial.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

  target_compile_definitions(ume_serial_client PUBLIC USE_SCORIA)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(ume_serial_client UmeScoria shm pthread)
  else()
    target_link_libraries(ume_serial_client UmeScoria shm pthread rt)
  endif()

  set_target_properties(ume_serial_client PROPERTIES
     COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
     COMPILE_OPTIONS "${WARNING_FLAGS}"
     )
endif()


#############################################################################
# Things that depend on MPI
#############################################################################
if(USE_MPI)
add_executable(ume_mpi ume_mpi.cc)
target_link_libraries(ume_mpi Ume)
set_target_properties(ume_mpi PROPERTIES
  COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
  COMPILE_OPTIONS "${WARNING_FLAGS}"
  )

if(USE_SCORIA)
  message(STATUS "Building Ume MPI with Scoria")
  add_executable(ume_mpi_client ume_mpi.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

  target_compile_definitions(ume_mpi_client PUBLIC USE_SCORIA)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(ume_mpi_client UmeScoria shm pthread)
  else()
    target_link_libraries(ume_mpi_client UmeScoria shm pthread rt)
  endif()

  set_target_properties(ume_mpi_client PROPERTIES
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
