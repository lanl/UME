# Ume/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

project(UME VERSION 1.0.0 LANGUAGES CXX)

# Add our cmake include directory to the search path
set(LAP_CMAKE_DIR "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${LAP_CMAKE_DIR} ${CMAKE_MODULE_PATH})

include(DisableIntree)

include(GNUInstallDirs)

include(BuildType)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

#include(pkgs/Catch2Support)
include(pkgs/DoxygenSupport)
include(pkgs/KokkosSupport)
include(pkgs/MPISupport)
include(pkgs/OpenACCSupport)

add_subdirectory(src)
