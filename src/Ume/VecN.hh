/*!
  \file VecN.hh
*/
#ifndef VECN_HH
#define VECN_HH 1

#include <algorithm>
#include <array>
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
  const_ref operator=(T const &v) {
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
  friend val operator+(val lhs, const_ref rhs) {
    lhs += rhs;
    return lhs;
  }
  friend val operator+(val lhs, T const &rhs) {
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
  friend val operator-(val lhs, const_ref rhs) {
    lhs -= rhs;
    return lhs;
  }
  friend val operator-(val lhs, T const &rhs) {
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
  friend val operator*(val lhs, const_ref rhs) {
    lhs *= rhs;
    return lhs;
  }
  friend val operator*(val lhs, T const &rhs) {
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
  bool operator==(const_ref rhs) const { return data_ == rhs.data_; }
  friend val operator/(val lhs, const_ref rhs) {
    lhs /= rhs;
    return lhs;
  }
  friend val operator/(val lhs, T const &rhs) {
    lhs /= rhs;
    return lhs;
  }
  friend std::ostream &operator<<(std::ostream &os, const_ref vec) {
    char const *delim = "";
    os << '<';
    for (T const &v : vec.data_) {
      os << std::exchange(delim, ", ") << v;
    }
    return os << '>';
  }

private:
  std::array<T, N> data_;
};

using Vec3 = VecN<double, 3>;

} // namespace Ume
#endif
