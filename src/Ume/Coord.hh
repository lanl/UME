/*!
  \file VecN.hh
*/
#ifndef VECN_HH
#define VECN_HH 1

#include <algorithm>
#include <array>

namespace Ume {

//! Mathematical vector type
template<class T, size_t N> class VecN {
public:
  using val = VecN<T,N>;
  using ref = val&;
  using const_ref = val const&;

public:
  const_ref operator=(T const &v) {
    data_.fill(v);
    return *this;
  }
  constexpr T& operator[](size_t const i) { return data_[i]; }
  constexpr T const & operator[](size_t const i) const { return data_[i]; }
  constexpr ref operator+=(const_ref rhs) {
    for(size_t i = 0; i < N; ++i) data_[i] += rhs.data_[i];
    return *this;
  }
  constexpr ref operator+=(T const &rhs) {
    for(size_t i = 0; i < N; ++i) data_[i] += rhs;
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
    for(size_t i = 0; i < N; ++i) data_[i] -= rhs.data_[i];
    return *this;
  }
  constexpr ref operator-=(T const &rhs) {
    for(size_t i = 0; i < N; ++i) data_[i] -= rhs;
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
    for(size_t i = 0; i < N; ++i) data_[i] *= rhs.data_[i];
    return *this;
  }
  constexpr ref operator*=(T const &rhs) {
    for(size_t i = 0; i < N; ++i) data_[i] *= rhs;
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
    for(size_t i = 0; i < N; ++i) data_[i] /= rhs.data_[i];
    return *this;
  }
  constexpr ref operator/=(T const &rhs) {
    for(size_t i = 0; i < N; ++i) data_[i] /= rhs;
    return *this;
  }
  friend val operator/(val lhs, const_ref rhs) {
    lhs /= rhs;
    return lhs;
  }
  friend val operator/(val lhs, T const &rhs) {
    lhs /= rhs;
    return lhs;
  }

private:
  std::array<T, N> data_;
};



using Coord3 = VecN<double, 3>;


} // namespace Ume
#endif
