/*
  \file Ds_Types.hh
*/

#ifndef DS_TYPES_HH
#define DS_TYPES_HH 1

#include "Ume/VecN.hh"
#include <variant>
#include <vector>

namespace Ume {

struct DS_Types {
  enum class Types { INT, INTV, DBL, DBLV, VEC3, VEC3V, NONE };
  using INT_T = int;
  using INTV_T = std::vector<INT_T>;
  using DBL_T = double;
  using DBLV_T = std::vector<DBL_T>;
  using VEC3_T = Vec3;
  using VEC3V_T = std::vector<VEC3_T>;
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
