# Ume/src/Ume/CMakeLists.txt

#############################################################################
# The basic Ume library
#############################################################################

set(UME_INCLUDE_FILES
  VecN.hh
  SOA_Idx_Mesh.hh
  )

add_library(ume_lib
  ${UME_INCLUDE_FILES}
  SOA_Idx_Mesh.cc
  )

set_target_properties(ume_lib PROPERTIES
  COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
  COMPILE_OPTIONS "${COMMON_COMPILE_OPTIONS}"
  OUTPUT_NAME "ume"
  )

target_include_directories(ume_lib
  PUBLIC
  $<BUILD_INTERFACE:${UME_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
  )

target_link_libraries(ume_lib
  PUBLIC
  ${COMMON_LINK_LIBRARIES}
  )


#############################################################################
# Ume executables
#############################################################################

add_executable(ume
  ume.cc
  )

set_target_properties(ume PROPERTIES
  COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
  COMPILE_OPTIONS "${COMMON_COMPILE_OPTIONS}"
  )

target_link_libraries(ume
  ume_lib
  )


#############################################################################
# Things that depend on Kokkos
#############################################################################

if(USE_KOKKOS)
  add_executable(kokkos_hello kokkos_hello.cc)

  set_target_properties(kokkos_hello PROPERTIES
    COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
    COMPILE_OPTIONS "${COMMON_COMPILE_OPTIONS}"
    )

  target_link_libraries(kokkos_hello
    ${KOKKOS_LINK_LIBRARIES}
    ${COMMON_LINK_LIBRARIES}
    )
endif()


#############################################################################
# Things that depend on OpenACC
#############################################################################

if(USE_OPENACC)
  add_executable(openacc_hello openacc_hello.cc)

  set_target_properties(openacc_hello PROPERTIES
    COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
    COMPILE_OPTIONS "${COMMON_COMPILE_OPTIONS}"
    )

  target_link_libraries(openacc_hello
    ${OPENACC_LINK_LIBRARIES}
    ${COMMON_LINK_LIBRARIES}
    )
endif()
