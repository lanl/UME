# Ume/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Add our cmake include directory to the search path
set(LAP_CMAKE_DIR "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${LAP_CMAKE_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(DisableIntree)

project(UME VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

include(BuildType)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Build option to force the use of the Kokkos::Serial backend.  This has to be
# set here, before any subdirectory is added, so that the library, the drivers
# and the tests all agree: UME_SERIAL changes the DevExecSpace/HostExecSpace
# typedefs in mem_exec_spaces.hh, and a mismatch between translation units is
# an ODR violation.
option(UME_SERIAL "Enable the Kokkos::Serial backend")
if (UME_SERIAL)
  add_compile_definitions(UME_SERIAL)
  message(STATUS "Using the Kokkos::Serial backend.")
endif()

include(pkgs/Catch2Support)
include(pkgs/DoxygenSupport)
include(pkgs/KokkosSupport)
include(pkgs/MPISupport)
include(pkgs/OpenACCSupport)
include(pkgs/libunwindSupport)

add_subdirectory(src)

# After src/, so that test/ can gate on the targets it builds and on the
# variables the pkgs/ modules above set.  Both are invisible from inside a
# pkgs/ module, where this used to live.
if(USE_CATCH2)
  add_subdirectory(test)
endif()
