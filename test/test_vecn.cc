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
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

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

TEST_CASE("V3 dotprod", "[vecN]") {
  Ume::Vec3 const a({1.0, 2.0, 4.0});
  Ume::Vec3 b({-10.0, 10.0, 10.0});
  REQUIRE(dotprod(a, b) == 50.0);
  b[0] *= -1;
  b[1] *= -1;
  REQUIRE(dotprod(a, b) == 30.0);
  b[1] *= -1;
  b[2] *= -1;
  REQUIRE(dotprod(a, b) == -10.0);
}

TEST_CASE("V3 crossprod", "[vecN]") {
  Ume::Vec3 a({1, 2, 3});
  Ume::Vec3 b({2, 3, 4});
  Ume::Vec3 cp = crossprod(a, b);
  REQUIRE(cp[0] == -1);
  REQUIRE(cp[1] == 2);
  REQUIRE(cp[2] == -1);
  cp = crossprod(b, a);
  REQUIRE(cp[0] == 1);
  REQUIRE(cp[1] == -2);
  REQUIRE(cp[2] == 1);
}

TEST_CASE("V3 normalize", "[vecN]") {
  Ume::Vec3 a({1, 1, 1});
  Ume::normalize(a);
  double mag = Ume::dotprod(a, a);
  REQUIRE_THAT(mag, Catch::Matchers::WithinULP(1.0, 1));
}

/* Scalar comparison */

TEST_CASE("Scalar == compares components as T", "[vecN]") {
  // Each component truncates to 0 as an int, and none of them is 0.0.
  Ume::Vec3 const fractional({0.4, -0.7, 0.9});
  REQUIRE_FALSE(fractional == 0.0);
  REQUIRE(fractional != 0.0);

  // Positive control: the all-zero vector is equal to 0.0, and moving one
  // component off zero by less than one flips the comparison.
  Ume::Vec3 v(0.0);
  REQUIRE(v == 0.0);
  REQUIRE_FALSE(v != 0.0);
  v[1] = 0.5;
  REQUIRE_FALSE(v == 0.0);
  REQUIRE(v != 0.0);

  // A non-integral scalar compares equal to itself.
  Ume::Vec3 const half(0.5);
  REQUIRE(half == 0.5);
  REQUIRE_FALSE(half != 0.5);
}

TEST_CASE("Scalar == on integer components", "[vecN]") {
  Ume::VecN<int, 2> const a({3, 3});
  REQUIRE(a == 3);
  REQUIRE_FALSE(a == 4);
  Ume::VecN<int, 2> const b({3, 4});
  REQUIRE_FALSE(b == 3);
  REQUIRE(b != 3);
}
