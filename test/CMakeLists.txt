# Ume/test/CMakeLists.txt

add_executable(ume_tests
  test_datastore.cc
  test_io.cc
  test_utils.cc
  test_vecn.cc
)

target_link_libraries(ume_tests PRIVATE
  Ume Catch2::Catch2WithMain)

catch_discover_tests(ume_tests)
