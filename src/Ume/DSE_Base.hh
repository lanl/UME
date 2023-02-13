/*!
  \file DSE_Base.hh
*/
#ifndef DSE_BASE_HH
#define DSE_BASE_HH 1

#include "Ume/Datastore.hh"
#include "Ume/SOA_Idx_Mesh.hh"

namespace Ume {
namespace SOA_Idx {

struct Corners;
struct Edges;
struct Faces;
struct Points;
struct Sides;
struct Zones;

//! A base class for Datastore entries
template <typename B> class DSE_Base : public DS_Entry {
public:
  DSE_Base(Types t, B &b) : DS_Entry(t), baseEnt_{b} {}
#define MAKE_DS_ACCESS(Y, R) \
  R &access_##Y(char const *const name) { \
    return baseEnt_.ds().access_##Y(name); \
  } \
  R const &caccess_##Y(char const *const name) const { \
    return baseEnt_.ds().caccess_##Y(name); \
  } \
  R &mydata_##Y() const { return std::get<R>(data_); }

  MAKE_DS_ACCESS(int, INT_T);
  MAKE_DS_ACCESS(intv, INTV_T);
  MAKE_DS_ACCESS(intrr, INTRR_T);
  MAKE_DS_ACCESS(dbl, DBL_T);
  MAKE_DS_ACCESS(dblv, DBLV_T);
  MAKE_DS_ACCESS(dblrr, DBLRR_T);
  MAKE_DS_ACCESS(vec3, VEC3_T);
  MAKE_DS_ACCESS(vec3v, VEC3V_T);
  MAKE_DS_ACCESS(vec3rr, VEC3RR_T);
#undef MAKE_DS_ACCESS

#define ENT_ACCESS(U, L) \
  U &L() { return baseEnt_.mesh().L; } \
  U &L() const { return baseEnt_.mesh().L; }

  ENT_ACCESS(Corners, corners)
  ENT_ACCESS(Edges, edges)
  ENT_ACCESS(Faces, faces)
  ENT_ACCESS(Points, points)
  ENT_ACCESS(Sides, sides)
  ENT_ACCESS(Zones, zones)
#undef ENT_ACCESS

  constexpr Mesh &mesh() { return baseEnt_.mesh(); }
  constexpr Mesh const &mesh() const { return baseEnt_.mesh(); }

  B &baseEnt_;
};

} // namespace SOA_Idx
} // namespace Ume

#endif
