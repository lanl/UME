# Ume/src/Ume/CMakeLists.txt

#############################################################################
# The basic Ume library
#############################################################################
#set(CMAKE_CXX_CLANG_TIDY
#  clang-tidy;
#  -header-filter=.;
#  -checks=-*,bugprone-*;)

set(UME_INCLUDE_FILES
  Comm_Buffers.hh
  Comm_MPI.hh
  Comm_Neighbors.hh
  Comm_Transport.hh
  DS_Types.hh
  Datastore.hh
  Entity_Field.hh
  Mesh_Base.hh
  RaggedRight.hh
  SOA_Entity.hh
  SOA_Idx_Corners.hh
  SOA_Idx_Edges.hh
  SOA_Idx_Faces.hh
  SOA_Idx_Mesh.hh
  SOA_Idx_Points.hh
  SOA_Idx_Sides.hh
  SOA_Idx_Zones.hh
  SOA_Idx_Iotas.hh
  Timer.hh
  VecN.hh
  face_area.hh
  gradient.hh
  renumbering.hh
  soa_idx_helpers.hh
  utils.hh
  memory.hh
  array_types.hh
  process_mgmt.hh
  mem_exec_spaces.hh
  )

add_library(Ume
  ${UME_INCLUDE_FILES}
  Comm_Buffers.cc
  Comm_MPI.cc
  Comm_Neighbors.cc
  Comm_Transport.cc
  Datastore.cc
  SOA_Entity.cc
  SOA_Idx_Corners.cc
  SOA_Idx_Edges.cc
  SOA_Idx_Faces.cc
  SOA_Idx_Mesh.cc
  SOA_Idx_Points.cc
  SOA_Idx_Sides.cc
  SOA_Idx_Zones.cc
  SOA_Idx_Iotas.cc
  face_area.cc
  gradient.cc
  renumbering.cc
  utils.cc
  process_mgmt.cc
  )

# Build option to force the use of the Kokkos::Serial backend.
if (UME_SERIAL)
  set(COMMON_COMPILE_DEFINITIONS
    ${COMMON_COMPILE_DEFINITIONS}
    UME_SERIAL
    )
  message(STATUS "Using the Kokkos::Serial backend.")
endif()

# Build option to compile and link with LLVM ASAN/UBSAN for
# developement/debugging purposes only.
if (UME_SANITIZE)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "LLVM Sanitizers require an LLVM-based compiler.")
  endif()
  target_compile_options(Ume PUBLIC
    -fno-omit-frame-pointer
    -fno-optimize-sibling-calls
    -fsanitize=address,undefined,integer,nullability
    -fsanitize-recover=address
  )
  target_link_options(Ume PUBLIC
    -fsanitize=address,undefined,integer,nullability
  )
  message(STATUS "Compiling and linking with ASAN and UBSAN.")
  message(STATUS "  Recommended to have the llvm-symbolizer in PATH.")
endif()

set_target_properties(Ume PROPERTIES
  COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
  COMPILE_OPTIONS "${COMMON_COMPILE_OPTIONS}"
  OUTPUT_NAME "ume"
  )

target_include_directories(Ume
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include/Ume>
  ${LIBUNWIND_INCLUDE_DIRS}
  )

target_link_libraries(Ume
  PUBLIC
  ${COMMON_LINK_LIBRARIES}
  PUBLIC
    Kokkos::kokkos
    )

target_compile_options(Ume
  PRIVATE ${WARNING_FLAGS}
  )

if (USE_KOKKOS)
  if (Kokkos_ENABLE_CUDA)
    target_compile_options(Ume PUBLIC "--expt-relaxed-constexpr")
  endif()
endif()

# -----------------------------------------------------------------------
# Library/Header installation section
# -----------------------------------------------------------------------

set(ConfigPackageLocation lib/cmake/Ume)

install(TARGETS Ume
  EXPORT UmeTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
  )

install(FILES ${UME_INCLUDE_FILES}
  DESTINATION include
  )

install(EXPORT UmeTargets
  FILE UmeTargets.cmake
  NAMESPACE Ume::
  DESTINATION ${ConfigPackageLocation}
  )

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    UmeConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
    )


configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/UmeConfig.cmake"
  INSTALL_DESTINATION ${ConfigPackageLocation}
)

configure_file(UmeConfig.cmake.in UmeConfig.cmake @ONLY)

export(EXPORT UmeTargets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/UmeTargets.cmake"
       NAMESPACE Ume::
)


install(FILES "${CMAKE_CURRENT_BINARY_DIR}/UmeConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/UmeConfigVersion.cmake"
        DESTINATION ${ConfigPackageLocation}
        )
