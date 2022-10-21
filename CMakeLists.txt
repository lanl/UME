# Ume/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Add our cmake include directory to the search path
set(LAP_CMAKE_DIR "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${LAP_CMAKE_DIR} ${CMAKE_MODULE_PATH})

include(DisableIntree)

project(UME LANGUAGES CXX)

include(BuildType)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include(pkgs/KokkosSupport)
include(pkgs/OpenACCSupport)

add_subdirectory(src)
