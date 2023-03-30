/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

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

TEST_CASE("size_t binary r/w", "[utils]") {
  const size_t in_size_t{1234567890123};
  size_t out_size_t;
  wr(in_size_t, out_size_t);
  REQUIRE(in_size_t == out_size_t);
}

TEST_CASE("int binary r/w", "[utils]") {
  const int in_int{-1234};
  int out_int;
  wr(in_int, out_int);
  REQUIRE(in_int == out_int);
}

TEST_CASE("std::string binary r/w", "[utils]") {
  const std::string in_str{"  This is a test  "};
  std::string out_str;
  wr(in_str, out_str);
  REQUIRE(in_str == out_str);
}

TEST_CASE("vector<int> binary r/w", "[utils]") {
  const std::vector<int> in_vec{-1, 1, -2, 2, -4, 4};
  std::vector<int> out_vec;
  wr(in_vec, out_vec);
  REQUIRE(in_vec == out_vec);
}

TEST_CASE("vector<short> binary r/w", "[utils]") {
  const std::vector<short> in_vec{-1, 1, -2, 2, -4, 4};
  std::vector<short> out_vec;
  wr(in_vec, out_vec);
  REQUIRE(in_vec == out_vec);
}

TEST_CASE("vector<Vec3> binary r/w", "[utils]") {
  using Ume::Vec3;
  std::vector<Vec3> in_vec{2}, out_vec;
  in_vec[0] = Vec3({1.0, 1.0e-12, 10});
  in_vec[1] = Vec3({-5.0, -5.0e-12, -50});
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
