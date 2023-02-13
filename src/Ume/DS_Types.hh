/*
  \file Ds_Types.hh
*/

#ifndef DS_TYPES_HH
#define DS_TYPES_HH 1

#include "Ume/VecN.hh"
#include <variant>
#include <vector>

namespace Ume {

template <class T> struct RaggedRight {
  std::vector<int> bidx, eidx;
  std::vector<T> data;
  RaggedRight() = default;
  explicit RaggedRight(int base_size) { init(base_size); }
  bool operator==(RaggedRight<T> const &rhs) const {
    return (begin == rhs.begin && end == rhs.end && data == rhs.data);
  }
  T *begin(int n) { return data.data() + bidx[n]; }
  T const *begin(int n) const { return data.data() + bidx[n]; }
  T *end(int n) { return data.data() + eidx[n]; }
  T const *end(int n) const { return data.data() + eidx[n]; }
  void init(int const n) {
    bidx.assign(n, 0);
    eidx.assign(n, 0);
  }
  template <class IT> void append(int n, IT b, IT e) {
    bidx.at(n) = static_cast<int>(data.size());
    data.insert(data.end(), b, e);
    eidx.at(n) = static_cast<int>(data.size());
  }
  constexpr int size(int const n) const { return eidx[n] - bidx[n]; }
};

struct DS_Types {
  enum class Types {
    INT,
    INTV,
    INTRR,
    DBL,
    DBLV,
    DBLRR,
    VEC3,
    VEC3V,
    VEC3RR,
    NONE
  };
  using INT_T = int;
  using INTV_T = std::vector<INT_T>;
  using INTRR_T = RaggedRight<INT_T>;
  using DBL_T = double;
  using DBLV_T = std::vector<DBL_T>;
  using DBLRR_T = RaggedRight<DBL_T>;
  using VEC3_T = Vec3;
  using VEC3V_T = std::vector<VEC3_T>;
  using VEC3RR_T = RaggedRight<VEC3_T>;
};

template <typename T> struct DS_Type_Info {};

template <> struct DS_Type_Info<DS_Types::INT_T> {
  using base_type = int;
  static const DS_Types::Types type{DS_Types::Types::INT};
  static unsigned const elem_len{1};
};

template <> struct DS_Type_Info<DS_Types::INTV_T> {
  using base_type = int;
  static const DS_Types::Types type{DS_Types::Types::INTV};
  static unsigned const elem_len{1};
};

template <> struct DS_Type_Info<DS_Types::DBL_T> {
  using base_type = double;
  static const DS_Types::Types type{DS_Types::Types::DBL};
  static unsigned const elem_len{1};
};

template <> struct DS_Type_Info<DS_Types::DBLV_T> {
  using base_type = double;
  static const DS_Types::Types type{DS_Types::Types::DBLV};
  static unsigned const elem_len{1};
};

template <> struct DS_Type_Info<DS_Types::VEC3_T> {
  using base_type = double;
  static const DS_Types::Types type{DS_Types::Types::VEC3};
  static int const elem_len{3};
};

template <> struct DS_Type_Info<DS_Types::VEC3V_T> {
  using base_type = double;
  static const DS_Types::Types type{DS_Types::Types::VEC3V};
  static unsigned const elem_len{3};
};

} // namespace Ume

#endif
