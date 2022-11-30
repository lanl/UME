# Ume/test/CMakeLists.txt

add_executable(ume_tests
  test_vecn.cc
  test_utils.cc)

target_link_libraries(ume_tests PRIVATE
  Ume Catch2::Catch2WithMain)

catch_discover_tests(ume_tests)
