/*!
  \file SOA_Idx_Mesh.hh
*/
#ifndef SOA_IDX_MESH_HH
#define SOA_IDX_MESH_HH 1

#include <array>
#include <iosfwd>
#include <vector>

namespace Ume {

//! A struct-of-arrays implementation using integer indexing
namespace SOA_Idx {

using PtCoord = std::array<double, 3>;

struct Entity {
  //! Mask flag
  std::vector<short> mask;
  /*! @name Minimal parallel connectivity

    These arrays define minimum inter-mesh connectivity.  Each entity listed in
    `cpy_idx` is a copy of an entity in a different rank.  The pair `{src_pe[i],
    src_idx[i]}` describes the address of the source entity for this copy.

    Copies of volumetric entries are called "ghosts", and they are generally not
    iterated across in a calculation, but are referenced via local connectivity.
    The source for a ghost is called a "real" entity. To simplify iteration, all
    ghosts are stored in the upper portion of the Entity indices: [lsize,
    size). Generally, ghost entities are only updated through scatter
    operations from sources to copies.

    Entities that are shared across processor boundaries are stored among the
    real entries in the index range [0, lsize).  The instance of a shared entity
    on some processor is designated the source; the rest are copies.  Some
    shared entities, such as faces, have one-to-one connectivity: for each
    source, there is one copy.  Other shared entities, such as points, are
    one-to-many: each source may have multiple copies. This occurs when an
    entity appears in multiple processor mesh domains.  Generally, all
    processors compute on shared entities, which are then combined with a
    gather-scatter paradigm, or simply a scatter from the src to ensure each
    partition is operating on the same value.

    This minimal connectivity is expressed in one direction: each copy knows
    where its source entity is.  This allows a simple representation, as this
    direction is always one-to-one.  But this means that a PE does not know what
    source entities reside locally, they only know the copies!
   */
  ///@{
  //! Local indices of copies
  std::vector<int> cpy_idx;
  //! The rank that owns the source entity
  std::vector<int> src_pe;
  //! The index of the source entity on the src_pe
  std::vector<int> src_idx;
  ///@}

  //! The number of local (non-ghost) entities
  int lsize = 0;

  virtual void write(std::ostream &os) const = 0;
  virtual void read(std::istream &is) = 0;
  int size() const { return static_cast<int>(mask.size()); }
};

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
};

//! SoA representation of mesh edges (connects two points)
struct Edges : public Entity {
  //! Indices of mesh points forming the endpoints
  std::vector<int> p1, p2;
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
};

//! SoA representation of mesh faces (separates zones)
struct Faces : public Entity {
  //! Indices of adjacent mesh zones
  std::vector<int> z1, z2;
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
};

//! Struct-of-Arrays (SoA) representation of mesh points
struct Points : public Entity {
  //! spatial coordinates of p
  std::vector<PtCoord> coord;
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
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
};

//! SoA representation of mesh zones
struct Zones : public Entity {
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
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
};

} // namespace SOA_Idx

} // namespace Ume

#endif
