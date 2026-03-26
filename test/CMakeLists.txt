# Ume/test/CMakeLists.txt

# UME standard tests

add_executable(ume_tests
  test_datastore.cc
  test_io.cc
  test_utils.cc
  test_vecn.cc
)

target_link_libraries(ume_tests PRIVATE
  Ume Catch2::Catch2WithMain)

catch_discover_tests(ume_tests)

# UME GPU-enabled tests

# Build option to force the use of the Kokkos::Serial backend.
option(UME_SERIAL "Enable the Kokkos::Serial backend")
if (UME_SERIAL)
  add_compile_definitions(UME_SERIAL)
endif()

add_executable(ume_gpu_tests
  test_scratch_arrays.cc
  custom_main.cc
)

target_link_libraries(ume_gpu_tests PRIVATE
  Ume Catch2::Catch2)

catch_discover_tests(ume_gpu_tests)
