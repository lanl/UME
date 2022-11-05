#include "Ume/VecN.hh"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Scalar Ctor", "[vecN]") {
  Ume::VecN<int, 2> v(2);
  REQUIRE(v[0] == 2);
  REQUIRE(v[1] == 2);
}

TEST_CASE("List Ctor", "[vecN]") {
  Ume::VecN<int, 2> v({3, 4});
  REQUIRE(v[0] == 3);
  REQUIRE(v[1] == 4);
}

TEST_CASE("Element Assignment", "[vecN]") {
  Ume::VecN<int, 2> v;
  v[0] = 3;
  v[1] = 4;
  REQUIRE(v[0] == 3);
  REQUIRE(v[1] == 4);
}

TEST_CASE("VecN Assignment", "[vecN]") {
  Ume::VecN<int, 2> a({3, 4});
  Ume::VecN<int, 2> b(0);
  b = a;
  REQUIRE(b[0] == 3);
  REQUIRE(b[1] == 4);
}

/* Addition */

TEST_CASE("VecN +=", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b({3, 4});
  a += b;
  REQUIRE(a[0] == 4);
  REQUIRE(a[1] == 6);
  REQUIRE(b[0] == 3);
  REQUIRE(b[1] == 4);
}

TEST_CASE("Scalar +=", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  a += 2;
  REQUIRE(a[0] == 3);
  REQUIRE(a[1] == 4);
}

TEST_CASE("VecN +", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b({3, 4});
  Ume::VecN<int, 2> c;
  c = a + b;
  REQUIRE(a[0] == 1);
  REQUIRE(a[1] == 2);
  REQUIRE(b[0] == 3);
  REQUIRE(b[1] == 4);
  REQUIRE(c[0] == 4);
  REQUIRE(c[1] == 6);
}

TEST_CASE("Scalar +", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b;
  b = a + 2;
  REQUIRE(a[0] == 1);
  REQUIRE(a[1] == 2);
  REQUIRE(b[0] == 3);
  REQUIRE(b[1] == 4);
}

/* Subtraction */

TEST_CASE("VecN -=", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b({3, 4});
  a -= b;
  REQUIRE(a[0] == -2);
  REQUIRE(a[1] == -2);
  REQUIRE(b[0] == 3);
  REQUIRE(b[1] == 4);
}

TEST_CASE("Scalar -=", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  a -= 2;
  REQUIRE(a[0] == -1);
  REQUIRE(a[1] == 0);
}

TEST_CASE("VecN -", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b({3, 4});
  Ume::VecN<int, 2> c;
  c = b - a;
  REQUIRE(a[0] == 1);
  REQUIRE(a[1] == 2);
  REQUIRE(b[0] == 3);
  REQUIRE(b[1] == 4);
  REQUIRE(c[0] == 2);
  REQUIRE(c[1] == 2);
}

TEST_CASE("Scalar -", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b;
  b = a - 2;
  REQUIRE(a[0] == 1);
  REQUIRE(a[1] == 2);
  REQUIRE(b[0] == -1);
  REQUIRE(b[1] == 0);
}

/* Multiplicaton */

TEST_CASE("VecN *=", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b({3, 4});
  a *= b;
  REQUIRE(a[0] == 3);
  REQUIRE(a[1] == 8);
  REQUIRE(b[0] == 3);
  REQUIRE(b[1] == 4);
}

TEST_CASE("Scalar *=", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  a *= 2;
  REQUIRE(a[0] == 2);
  REQUIRE(a[1] == 4);
}

TEST_CASE("VecN *", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b({3, 4});
  Ume::VecN<int, 2> c;
  c = a * b;
  REQUIRE(a[0] == 1);
  REQUIRE(a[1] == 2);
  REQUIRE(b[0] == 3);
  REQUIRE(b[1] == 4);
  REQUIRE(c[0] == 3);
  REQUIRE(c[1] == 8);
}

TEST_CASE("Scalar *", "[vecN]") {
  Ume::VecN<int, 2> a({1, 2});
  Ume::VecN<int, 2> b;
  b = a * 2;
  REQUIRE(a[0] == 1);
  REQUIRE(a[1] == 2);
  REQUIRE(b[0] == 2);
  REQUIRE(b[1] == 4);
}

/* Division */

TEST_CASE("VecN /=", "[vecN]") {
  Ume::VecN<double, 2> a({8, 4});
  Ume::VecN<double, 2> b({2, 4});
  a /= b;
  REQUIRE(a[0] == 4);
  REQUIRE(a[1] == 1);
  REQUIRE(b[0] == 2);
  REQUIRE(b[1] == 4);
}

TEST_CASE("Scalar /=", "[vecN]") {
  Ume::VecN<int, 2> a({8, 4});
  a /= 2;
  REQUIRE(a[0] == 4);
  REQUIRE(a[1] == 2);
}

TEST_CASE("VecN /", "[vecN]") {
  Ume::VecN<double, 3> a({16, 8, 4});
  Ume::VecN<double, 3> b({4, 2, 1});
  Ume::VecN<double, 3> c;
  c = a / b;
  REQUIRE(c[0] == 4);
  REQUIRE(c[1] == 4);
  REQUIRE(c[2] == 4);
}

TEST_CASE("Scalar /", "[vecN]") {
  Ume::VecN<int, 2> a({4, 2});
  Ume::VecN<int, 2> b;
  b = a / 2;
  REQUIRE(b[0] == 2);
  REQUIRE(b[1] == 1);
}
