# Ume/src/Ume/CMakeLists.txt

#############################################################################
# The basic Ume library
#############################################################################
#set(CMAKE_CXX_CLANG_TIDY
#  clang-tidy;
#  -header-filter=.;
#  -checks=-*,bugprone-*;)

set(UME_INCLUDE_FILES
  Comm_MPI.hh
  Comm_Neighbors.hh
  Comm_Transport.hh
  Datastore.hh
  SOA_Entity.hh
  SOA_Idx_Corners.hh
  SOA_Idx_Edges.hh
  SOA_Idx_Faces.hh
  SOA_Idx_Mesh.hh
  SOA_Idx_Points.hh
  SOA_Idx_Sides.hh
  SOA_Idx_Zones.hh
  soa_idx_helpers.hh
  Timer.hh
  VecN.hh
  utils.hh
  )

add_library(Ume
  ${UME_INCLUDE_FILES}
  Comm_MPI.cc
  Comm_Neighbors.cc
  Datastore.cc
  SOA_Entity.cc
  SOA_Idx_Corners.cc
  SOA_Idx_Edges.cc
  SOA_Idx_Faces.cc
  SOA_Idx_Mesh.cc
  SOA_Idx_Points.cc
  SOA_Idx_Sides.cc
  SOA_Idx_Zones.cc
  )

set_target_properties(Ume PROPERTIES
  COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
  COMPILE_OPTIONS "${COMMON_COMPILE_OPTIONS}"
  OUTPUT_NAME "ume"
  )

target_include_directories(Ume
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
  )

target_link_libraries(Ume
  PUBLIC
  ${COMMON_LINK_LIBRARIES}
  )

target_compile_options(Ume
  PRIVATE ${WARNING_FLAGS}
  )

install(TARGETS Ume
  EXPORT UmeTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Ume
  )

install(FILES ${UME_INCLUDE_FILES}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Ume
  )

install(EXPORT UmeTargets
  FILE UmeTargets.cmake
  NAMESPACE Ume::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Ume
  )

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/UmeConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Ume
)

export(EXPORT UmeTargets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/UmeTargets.cmake"
       NAMESPACE Ume::
)
