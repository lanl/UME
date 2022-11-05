option(USE_CATCH2 "Enable support for Catch2 testing" ON)

if (USE_CATCH2)
  find_package(Catch2 3 REQUIRED)
  include(CTest)
  include(Catch)
  add_subdirectory(test)
endif()
