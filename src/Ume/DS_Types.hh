/*!
  \file Ume/DS_Types.hh
*/

#ifndef UME_DS_TYPES_HH
#define UME_DS_TYPES_HH 1

#include "Ume/RaggedRight.hh"
#include "Ume/VecN.hh"
#include <variant>
#include <vector>

namespace Ume {

//! Types that can be held in the datastore
struct DS_Types {
  //! An enumeration for switching between types
  enum class Types {
    INT, //!< scalar integer
    INTV, //!< vector<int>
    INTRR, //!< RaggedRight<int>
    DBL, //!< scalar double
    DBLV, //!< vector<double>
    DBLRR, //!< RaggedRight<double>
    VEC3, //!< scalar Vec3
    VEC3V, //!< vector<Vec3>
    VEC3RR, //!< RaggedRight<Vec3>
    NONE //!< Placeholder
  };
  // The actual C++ type declarations
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

//! A way of providing type information
/*! There is a template specialization provided below for each of the DS_Types
 */
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
