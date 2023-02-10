#include "Ume/DS_Types.hh"
#include "Ume/utils.hh"

#include <catch2/catch_test_macros.hpp>
#include <sstream>

using INT_T = Ume::DS_Types::INT_T;
using INTV_T = Ume::DS_Types::INTV_T;
using DBL_T = Ume::DS_Types::DBL_T;
using DBLV_T = Ume::DS_Types::DBLV_T;
using VEC3V_T = Ume::DS_Types::VEC3V_T;
using VEC3_T = Ume::DS_Types::VEC3_T;

TEST_CASE("string", "[IO]") {
  std::string data_out, data_in;
  data_out = "This is a test of a string\nWith some stuff";
  std::stringstream iobuf;
  Ume::write_bin(iobuf, data_out);
  Ume::read_bin(iobuf, data_in);
  REQUIRE(data_out == data_in);
}

TEST_CASE("vector<int>", "[IO]") {
  INTV_T data_out, data_in;
  int const n = 213;
  for (int i = 0; i < n; ++i) {
    data_out.emplace_back(INT_T(i));
  }
  std::stringstream iobuf;
  Ume::write_bin(iobuf, data_out);
  Ume::read_bin(iobuf, data_in);
  REQUIRE(data_out == data_in);
}

TEST_CASE("vector<dbl>", "[IO]") {
  DBLV_T data_out, data_in;
  int const n = 213;
  for (int i = 0; i < n; ++i) {
    data_out.emplace_back(DBL_T(i));
  }
  std::stringstream iobuf;
  Ume::write_bin(iobuf, data_out);
  Ume::read_bin(iobuf, data_in);
  REQUIRE(data_out == data_in);
}

TEST_CASE("vector<vecn>", "[IO]") {
  VEC3V_T data_out, data_in;
  for (int i = 0; i < 200; ++i) {
    data_out.emplace_back(VEC3_T(i));
  }
  std::stringstream iobuf;
  Ume::write_bin(iobuf, data_out);
  Ume::read_bin(iobuf, data_in);
  REQUIRE(data_out == data_in);
}
