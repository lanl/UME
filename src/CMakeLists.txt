# Ume/src/CMakeLists.txt

add_subdirectory(Ume)

add_executable(ume_driver ume_driver.cc)
target_link_libraries(ume_driver Ume)
set_target_properties(ume_driver PROPERTIES
  COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
  COMPILE_OPTIONS "${WARNING_FLAGS}"
  )


add_executable(txt2bin txt2bin.cc)
target_link_libraries(txt2bin Ume)
target_compile_options(txt2bin PRIVATE ${WARNING_FLAGS}
  ${COMMON_COMPILE_OPTIONS})

add_executable(parallel parallel.cc)
target_link_libraries(parallel Ume)
target_compile_options(parallel PRIVATE ${WARNING_FLAGS}
  ${COMMON_COMPILE_OPTIONS})


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
