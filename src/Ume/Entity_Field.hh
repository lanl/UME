/*!
  \file Ume/Entity_Field.hh
*/
#ifndef UME_ENTITY_FIELD_HH
#define UME_ENTITY_FIELD_HH 1

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

//! A base class for entity field variables in the Datastore
/*! This extends DS_Entry with access/caccess members for retrieving other data
    when intializing the variable, and adds accessors for Mesh and entity
    information. */
template <typename Entity> class Entity_Field : public DS_Entry {
public:
  //! Associate this field with an Entity and set the datatype
  Entity_Field(Types t, Entity &be) : DS_Entry(t), base_entity_{be} {}

  //! Define Datastore accessors via the base entity and an accessor for mydata
#define MAKE_DS_ACCESS(Y, R) \
  R &access_##Y(char const *const name) { \
    return base_entity_.ds().access_##Y(name); \
  } \
  R const &caccess_##Y(char const *const name) const { \
    return base_entity_.ds().caccess_##Y(name); \
  } \
  R &mydata_##Y() const { return std::get<R>(data_); }

  //! Datastore accessors for scalar integer
  MAKE_DS_ACCESS(int, INT_T);
  //! Datastore accessors for vector<int>
  MAKE_DS_ACCESS(intv, INTV_T);
  //! Datastore accessors for RaggedRight<int>
  MAKE_DS_ACCESS(intrr, INTRR_T);
  //! Datastore accessors for scalar double
  MAKE_DS_ACCESS(dbl, DBL_T);
  //! Datastore accessors for vector<double>
  MAKE_DS_ACCESS(dblv, DBLV_T);
  //! Datastore accessors for RaggedRight<double>
  MAKE_DS_ACCESS(dblrr, DBLRR_T);
  //! Datastore accessors for scalar Vec3
  MAKE_DS_ACCESS(vec3, VEC3_T);
  //! Datastore accessors for vector<Vec3>
  MAKE_DS_ACCESS(vec3v, VEC3V_T);
  //! Datastore accessors for RaggedRight<Vec3>
  MAKE_DS_ACCESS(vec3rr, VEC3RR_T);
#undef MAKE_DS_ACCESS

  //! Define normal and const Entity accessors
#define ENT_ACCESS(U, L) \
  U &L() { return base_entity_.mesh().L; } \
  U &L() const { return base_entity_.mesh().L; }

  //! Access the Corners of this mesh
  ENT_ACCESS(Corners, corners)
  //! Access the Edges of this mesh
  ENT_ACCESS(Edges, edges)
  //! Access the Faces of this mesh
  ENT_ACCESS(Faces, faces)
  //! Access the Points of this mesh
  ENT_ACCESS(Points, points)
  //! Access the Sides of this mesh
  ENT_ACCESS(Sides, sides)
  //! Acess the Zones of this mesh
  ENT_ACCESS(Zones, zones)
#undef ENT_ACCESS

  //! Mesh accessor
  constexpr Mesh &mesh() { return base_entity_.mesh(); }
  //! Const mesh accessor
  constexpr Mesh const &mesh() const { return base_entity_.mesh(); }

private:
  Entity &base_entity_;
};

} // namespace SOA_Idx
} // namespace Ume

#endif
