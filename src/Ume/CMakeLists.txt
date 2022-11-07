# Ume/src/Ume/CMakeLists.txt

#############################################################################
# The basic Ume library
#############################################################################

set(UME_INCLUDE_FILES
  SOA_Idx_Mesh.hh
  Timer.hh
  VecN.hh
  )

add_library(Ume
  ${UME_INCLUDE_FILES}
  SOA_Idx_Mesh.cc
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
