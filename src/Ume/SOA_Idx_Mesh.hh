/*!
  \file SOA_Idx_Mesh.hh
*/
#ifndef SOA_IDX_MESH_HH
#define SOA_IDX_MESH_HH 1

#include "Ume/Datastore.hh"
#include "Ume/SOA_Entity.hh"
#include "Ume/SOA_Idx_Faces.hh"
#include "Ume/SOA_Idx_Sides.hh"
#include "Ume/SOA_Idx_Zones.hh"
#include "Ume/VecN.hh"
#include <iosfwd>
#include <vector>

namespace Ume {

//! A struct-of-arrays implementation using integer indexing
namespace SOA_Idx {

using PtCoord = Vec3;
using dsptr = Ume::Datastore::dsptr;
using Types = Ume::DS_Types::Types;

//! SoA representation of mesh corners
/*!
  A corner is a subzonal volume defined by a zone centroid, a point
  on that zone, and the midpoints of the zone faces adjacent to that
  point, and the midpoints of the zone edges adjacent to that point.
  On a hexahedral mesh, a corner is a hexagon as well.
*/
struct Corners : public Entity {
public:
  Corners() = delete;
  explicit Corners(Mesh *mesh);
  void write(std::ostream &os) const override;
  void write(dsptr ds, std::ostream &os) const;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Corners const &rhs) const;
};

//! SoA representation of mesh edges (connects two points)
struct Edges : public Entity {
  explicit Edges(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Edges const &rhs) const;
};

//! Struct-of-Arrays (SoA) representation of mesh points
struct Points : public Entity {
  explicit Points(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Points const &rhs) const;
};

struct Mesh {
  enum Geometry_Type { CARTESIAN, CYLINDRICAL, SPHERICAL };
  int mype;
  int numpe;
  Geometry_Type geo;
  Datastore::dsptr ds;
  Corners corners;
  Edges edges;
  Faces faces;
  Points points;
  Sides sides;
  Zones zones;
  Mesh();
  void write(std::ostream &os) const;
  void read(std::istream &is);
  constexpr size_t ndims() const { return 3; }
  bool operator==(Mesh const &rhs) const;
  void print_stats(std::ostream &os) const;
};

} // namespace SOA_Idx

} // namespace Ume

#endif
