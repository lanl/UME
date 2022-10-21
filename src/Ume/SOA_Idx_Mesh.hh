#ifndef SOA_IDX_MESH_HH
#define SOA_IDX_MESH_HH 1

#include <iosfwd>
#include <valarray>
#include <vector>

#include "Ume/Pt.hh"

namespace Ume {

//! A struct-of-arrays implementation using integer indexing
namespace SOA_Idx {

//! SoA representation of mesh corners
/*!
  A corner is a subzonal volume defined by a zone centroid, a point
  on that zone, and the midpoints of the zone faces adjacent to that
  point, and the midpoints of the zone edges adjacent to that point.
  On a hexahedral mesh, a corner is a hexagon as well.
*/
struct Corners {
  //! Mask flagb
  std::vector<short> mask;
  //! Index of characteristic mesh point
  std::vector<int> p;
  //! Index of parent mesh zone
  std::vector<int> z;
  size_t size() const { return mask.size(); }
};


//! SoA representation of mesh edges (connects two points)
struct Edges
{
  //! Mask flag
  std::vector<short> mask;
  //! Indices of mesh points forming the endpoints
  std::vector<int> p1, p2;
  void resize(size_t n);
  void write(std::ostream& os) const;
  void read(std::istream& is);
  bool operator==(Edges const & rhs) const;
  size_t size() const { return mask.size(); }
};


//! SoA representation of mesh faces (separates zones)
struct Faces
{
  //! Mask flag
  std::vector<short> mask;
  //! Indices of adjacent mesh zones
  std::vector<int> z1, z2;
  void resize(size_t n);
  void write(std::ostream& os) const;
  void read(std::istream& is);
  bool operator==(Faces const & rhs) const;
  size_t size() const { return mask.size(); }
};


//! Struct-of-Arrays (SoA) representation of mesh points
template<unsigned D> struct Points
{
  //! spatial coordinates of p
  std::vector<Pt<D>> coord;
  //! Mask flag
  std::vector<short> mask;
  //! MPI flag type
  std::vector<short> kkmpiptyp;
  void resize(int ndims, size_t n);;
  void write(std::ostream& os) const;
  void read(std::istream& is);
  bool operator==(Points const & rhs) const;
  size_t size() const { return mask.size(); }
};


//! SoA representation of mesh sides
/*! A side is another subzonal quantity, formed by a zone centroid,
    the centroid of a face on that zone, and an edge on that face.  On
    a hexahedral mesh, a side is a tetrahedron.  The side is the
    principal entity for volumetric calculations, so there is a lot of
    additional connectivity information carried here. */
struct Sides
{
  //! Mask flag
  std::vector<short> mask;
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
  void resize(size_t n);
  size_t size() const { return mask.size(); }
};

//! SoA representation of mesh zones
struct Zones
{
  //! Mask array
  std::vector<short> mask;
  void resize(size_t n);
  void write(std::ostream& os) const;
  void read(std::istream& is);
  bool operator==(Zones const & rhs) const;
  size_t size() const { return mask.size(); }
};


template<unsigned D> struct Mesh
{
  enum Geometry_Type {CARTESIAN, CYLINDRICAL, SPHERICAL};
  int mype;
  int numpe;
  Geometry_Type geo;
  Points<D> points;
  Edges edges;
  Faces faces;
  Sides sides;
  Corners corners;
  Zones zones;
  void write(std::ostream& os) const;
  void read(std::istream& is);
  bool operator==(Mesh const &) const;
  void print_stats(std::ostream& os) const;
  constexpr size_t ndims() const { return D; }
};



} // SOA_Idx

} // Ume

#endif
