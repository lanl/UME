/*!
  \file SOA_Idx_Mesh.hh
*/
#ifndef SOA_IDX_MESH_HH
#define SOA_IDX_MESH_HH 1

#include "Ume/SOA_Entity.hh"
#include "Ume/VecN.hh"
#include <iosfwd>
#include <vector>

namespace Ume {

//! A struct-of-arrays implementation using integer indexing
namespace SOA_Idx {

using PtCoord = Vec3;

//! SoA representation of mesh corners
/*!
  A corner is a subzonal volume defined by a zone centroid, a point
  on that zone, and the midpoints of the zone faces adjacent to that
  point, and the midpoints of the zone edges adjacent to that point.
  On a hexahedral mesh, a corner is a hexagon as well.
*/
struct Corners : public Entity {
  Corners() = default;
  //! Index of characteristic mesh point
  std::vector<int> p;
  //! Index of parent mesh zone
  std::vector<int> z;
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Corners const &rhs) const;
};

//! SoA representation of mesh edges (connects two points)
struct Edges : public Entity {
  //! Indices of mesh points forming the endpoints
  std::vector<int> p1, p2;
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Edges const &rhs) const;
};

//! SoA representation of mesh faces (separates zones)
struct Faces : public Entity {
  //! Indices of adjacent mesh zones
  std::vector<int> z1, z2;
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Faces const &rhs) const;
};

//! Struct-of-Arrays (SoA) representation of mesh points
struct Points : public Entity {
  //! spatial coordinates of p
  std::vector<PtCoord> coord;
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Points const &rhs) const;
};

//! SoA representation of mesh sides
/*! A side is another subzonal quantity, formed by a zone centroid,
    the centroid of a face on that zone, and an edge on that face.  On
    a hexahedral mesh, a side is a tetrahedron.  The side is the
    principal entity for volumetric calculations, so there is a lot of
    additional connectivity information carried here. */
struct Sides : public Entity {
  //! The index of the parent mesh zone
  std::vector<int> z;
  //! The indices of the points of 'e' (note, redundant, but heavily used)
  std::vector<int> p1, p2;
  //! The index of the mesh edge connecting p1, p2
  std::vector<int> e;
  //! The index of the mesh face of z that contains e
  std::vector<int> f;
  //! The indices of the corners of z that this side intersects
  std::vector<int> c1, c2;
  /*! The indices of the sides adjacent to this one.  Note that one of these
    will belong to another zone. */
  std::vector<int> s2, s3, s4, s5;
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Sides const &rhs) const;
};

//! SoA representation of mesh zones
struct Zones : public Entity {
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Zones const &rhs) const;
};

struct Mesh {
  enum Geometry_Type { CARTESIAN, CYLINDRICAL, SPHERICAL };
  int mype;
  int numpe;
  Geometry_Type geo;
  Points points;
  Edges edges;
  Faces faces;
  Sides sides;
  Corners corners;
  Zones zones;
  void write(std::ostream &os) const;
  void read(std::istream &is);
  constexpr size_t ndims() const { return 3; }
  bool operator==(Mesh const &rhs) const;
  void print_stats(std::ostream &os) const;
};

} // namespace SOA_Idx

} // namespace Ume

#endif
