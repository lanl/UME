#include "Ume/VecN.hh"
#include "Ume/utils.hh"
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>
#include <vector>

template <class T> void wr(T const &in_val, T &out_val) {
  std::stringstream filebuf;
  Ume::write_bin(filebuf, in_val);
  Ume::read_bin(filebuf, out_val);
}

TEST_CASE("Scalar R/W size_t", "[utils]") {
  const size_t in_size_t{1234567890123};
  size_t out_size_t;
  wr(in_size_t, out_size_t);
  REQUIRE(in_size_t == out_size_t);
}

TEST_CASE("Scalar R/W int", "[utils]") {
  const int in_int{-1234};
  int out_int;
  wr(in_int, out_int);
  REQUIRE(in_int == out_int);
}

TEST_CASE("Scalar R/W std::string", "[utils]") {
  const std::string in_str{"  This is a test  "};
  std::string out_str;
  wr(in_str, out_str);
  REQUIRE(in_str == out_str);
}

TEST_CASE("std::vector<int> R/W", "[utils]") {
  const std::vector<int> in_vec{-1, 1, -2, 2, -4, 4};
  std::vector<int> out_vec;
  wr(in_vec, out_vec);
  REQUIRE(in_vec == out_vec);
}

TEST_CASE("std::vector<Coord3> R/W", "[utils]") {
  using Ume::Coord3;
  std::vector<Coord3> in_vec{2}, out_vec;
  in_vec[0] = Coord3({1.0, 1.0e-12, 10});
  in_vec[1] = Coord3({-5.0, -5.0e-12, -50});
  wr(in_vec, out_vec);
  REQUIRE(in_vec == out_vec);
}

TEST_CASE("ltrim", "[utils]") {
  std::string a{"  \tThis is a test\t    "};
  std::string b;
  b = Ume::ltrim(a);
  REQUIRE(b == "This is a test\t    ");
}

TEST_CASE("rtrim", "[utils]") {
  std::string a{"  \tThis is a test\t    "};
  std::string b;
  b = Ume::rtrim(a);
  REQUIRE(b == "  \tThis is a test");
}

TEST_CASE("trim", "[utils]") {
  std::string a{"  \tThis is a test\t    "};
  std::string b;
  b = Ume::trim(a);
  REQUIRE(b == "This is a test");
}
