/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*!
  \file Ume/VecN.hh
*/
#ifndef UME_VECN_HH
#define UME_VECN_HH 1

#include <algorithm>
#include <array>
#include <cmath>
#include <compare>
#include <ostream>
#include <type_traits>
#include <utility>

namespace Ume {

//! Mathematical vector type
template <class T, size_t N> class VecN {
public:
  using val = VecN<T, N>;
  using ref = val &;
  using const_ref = val const &;

public:
  constexpr VecN() {
    static_assert(std::is_arithmetic_v<T>, "VecN is only for arithmetic types");
  }
  explicit constexpr VecN(T const &val) {
    static_assert(std::is_arithmetic_v<T>, "VecN is only for arithmetic types");
    data_.fill(val);
  }
  explicit constexpr VecN(std::array<T, N> &&val) : data_{val} {
    static_assert(std::is_arithmetic_v<T>, "VecN is only for arithmetic types");
  }
  constexpr const_ref operator=(T const &v) {
    data_.fill(v);
    return *this;
  }
  constexpr T &operator[](size_t const i) { return data_[i]; }
  constexpr T const &operator[](size_t const i) const { return data_[i]; }
  constexpr ref operator+=(const_ref rhs) {
    for (size_t i = 0; i < N; ++i)
      data_[i] += rhs.data_[i];
    return *this;
  }
  constexpr ref operator+=(T const &rhs) {
    for (size_t i = 0; i < N; ++i)
      data_[i] += rhs;
    return *this;
  }
  friend constexpr val operator+(val lhs, const_ref rhs) {
    lhs += rhs;
    return lhs;
  }
  friend constexpr val operator+(val lhs, T const &rhs) {
    lhs += rhs;
    return lhs;
  }
  constexpr ref operator-=(const_ref rhs) {
    for (size_t i = 0; i < N; ++i)
      data_[i] -= rhs.data_[i];
    return *this;
  }
  constexpr ref operator-=(T const &rhs) {
    for (size_t i = 0; i < N; ++i)
      data_[i] -= rhs;
    return *this;
  }
  friend constexpr val operator-(val lhs, const_ref rhs) {
    lhs -= rhs;
    return lhs;
  }
  friend constexpr val operator-(val lhs, T const &rhs) {
    lhs -= rhs;
    return lhs;
  }
  constexpr ref operator*=(const_ref rhs) {
    for (size_t i = 0; i < N; ++i)
      data_[i] *= rhs.data_[i];
    return *this;
  }
  constexpr ref operator*=(T const &rhs) {
    for (size_t i = 0; i < N; ++i)
      data_[i] *= rhs;
    return *this;
  }
  friend constexpr val operator*(val lhs, const_ref rhs) {
    lhs *= rhs;
    return lhs;
  }
  friend constexpr val operator*(val lhs, T const &rhs) {
    lhs *= rhs;
    return lhs;
  }
  constexpr ref operator/=(const_ref rhs) {
    for (size_t i = 0; i < N; ++i)
      data_[i] /= rhs.data_[i];
    return *this;
  }
  constexpr ref operator/=(T const &rhs) {
    for (size_t i = 0; i < N; ++i)
      data_[i] /= rhs;
    return *this;
  }
  friend constexpr val operator/(val lhs, const_ref rhs) {
    lhs /= rhs;
    return lhs;
  }
  friend constexpr val operator/(val lhs, T const &rhs) {
    lhs /= rhs;
    return lhs;
  }

  constexpr auto operator<=>(const_ref rhs) const = default;
  constexpr bool operator==(const_ref rhs) const = default;
  constexpr bool operator==(T const &rhs) const {
    return std::all_of(
        data_.begin(), data_.end(), [rhs](int i) { return i == rhs; });
  }
  constexpr bool operator!=(T const &rhs) const { return !(*this == rhs); }

  friend std::ostream &operator<<(std::ostream &os, const_ref vec) {
    char const *delim = "";
    os << '<';
    for (T const &v : vec.data_) {
      os << std::exchange(delim, ", ") << v;
    }
    return os << '>';
  }
  constexpr auto begin() { return data_.begin(); }
  constexpr auto end() { return data_.end(); }
  constexpr auto begin() const { return data_.begin(); }
  constexpr auto end() const { return data_.end(); }
  constexpr auto cbegin() { return data_.cbegin(); }
  constexpr auto cend() { return data_.cend(); }

private:
  std::array<T, N> data_;
};

using Vec3 = VecN<double, 3>;

constexpr Vec3 crossprod(Vec3 const &a, Vec3 const &b) {
  return Vec3{{a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
      a[0] * b[1] - a[1] * b[0]}};
}

constexpr double dotprod(Vec3 const &a, Vec3 const &b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline void normalize(Vec3 &a) {
  double const mag = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
  a /= std::sqrt(mag);
}

constexpr double vectormag(Vec3 const &a) {
  double const mag = std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
  return mag;
}

} // namespace Ume
#endif
