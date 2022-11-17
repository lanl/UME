/*!
  \file SOA_Idx_Mesh.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/utils.hh"
#include <istream>
#include <ostream>

namespace {
//! Scalar binary write
template <class T> inline void write_bin(std::ostream &os, T const &data) {
  os.write(reinterpret_cast<char const *>(&data), sizeof(T));
}

//! Scalar binary read
template <class T> inline void read_bin(std::istream &is, T &data) {
  is.read(reinterpret_cast<char *>(&data), sizeof(T));
}

//! Binary write for std::vector
template <class T>
void write_bin(std::ostream &os, std::vector<T> const &data) {
  write_bin(os, data.size());
  if (!data.empty()) {
    os.write(reinterpret_cast<const char *>(&data[0]), sizeof(T) * data.size());
  }
  os << '\n';
}

//! Binary read for std::vector
template <class T> void read_bin(std::istream &is, std::vector<T> &data) {
  size_t len;
  read_bin(is, len);
  if (len == 0) {
    data.clear();
  } else {
    data.resize(len);
    is.read(reinterpret_cast<char *>(&data[0]), sizeof(T) * len);
  }
  Ume::skip_line(is);
}
} // namespace

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Entity -------------------------------- */

void Entity::write(std::ostream &os) const {
  write_bin(os, lsize);
  write_bin(os, mask);
  write_bin(os, comm_type);
  write_bin(os, cpy_idx);
  write_bin(os, src_pe);
  write_bin(os, src_idx);
  write_bin(os, ghost_mask);
  os << '\n';
}

void Entity::read(std::istream &is) {
  read_bin(is, lsize);
  read_bin(is, mask);
  read_bin(is, comm_type);
  read_bin(is, cpy_idx);
  read_bin(is, src_pe);
  read_bin(is, src_idx);
  read_bin(is, ghost_mask);
  skip_line(is);
}

bool Entity::operator==(Entity const &rhs) const {
  return (lsize == rhs.lsize && mask == rhs.mask &&
      comm_type == rhs.comm_type && cpy_idx == rhs.cpy_idx &&
      src_pe == rhs.src_pe && src_idx == rhs.src_idx &&
      ghost_mask == rhs.ghost_mask);
}

void Entity::resize(int const local, int const total, int const ghost) {
  mask.resize(total);
  comm_type.resize(total);
  cpy_idx.resize(ghost);
  src_pe.resize(ghost);
  src_idx.resize(ghost);
  ghost_mask.resize(ghost);
  lsize = local;
}

/* -------------------------------- Corners ---------------------------------*/

void Corners::write(std::ostream &os) const {
  Entity::write(os);
  write_bin(os, p);
  write_bin(os, z);
  os << '\n';
}

void Corners::read(std::istream &is) {
  Entity::read(is);
  read_bin(is, p);
  read_bin(is, z);
  skip_line(is);
}

bool Corners::operator==(Corners const &rhs) const {
  return (Entity::operator==(rhs) && p == rhs.p && z == rhs.z);
}

void Corners::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  p.resize(total);
  z.resize(total);
}

/* --------------------------------- Edges ----------------------------------*/

void Edges::write(std::ostream &os) const {
  Entity::write(os);
  write_bin(os, p1);
  write_bin(os, p2);
  os << '\n';
}

void Edges::read(std::istream &is) {
  Entity::read(is);
  read_bin(is, p1);
  read_bin(is, p2);
  skip_line(is);
}

bool Edges::operator==(Edges const &rhs) const {
  return (Entity::operator==(rhs) && p1 == rhs.p1 && p2 == rhs.p2);
}

void Edges::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  p1.resize(total);
  p2.resize(total);
}

/* --------------------------------- Faces ----------------------------------*/

void Faces::write(std::ostream &os) const {
  Entity::write(os);
  write_bin(os, z1);
  write_bin(os, z2);
  os << '\n';
}

void Faces::read(std::istream &is) {
  Entity::read(is);
  read_bin(is, z1);
  read_bin(is, z2);
  skip_line(is);
}

bool Faces::operator==(Faces const &rhs) const {
  return (Entity::operator==(rhs) && z1 == rhs.z1 && z2 == rhs.z2);
}
void Faces::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  z1.resize(total);
  z2.resize(total);
}

/* --------------------------------- Points ---------------------------------*/

void Points::write(std::ostream &os) const {
  Entity::write(os);
  write_bin(os, coord);
  os << '\n';
}

void Points::read(std::istream &is) {
  Entity::read(is);
  read_bin(is, coord);
  skip_line(is);
}

bool Points::operator==(Points const &rhs) const {
  return (Entity::operator==(rhs) && coord == rhs.coord);
}
void Points::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  coord.resize(total);
}

/* --------------------------------- Sides ----------------------------------*/

void Sides::write(std::ostream &os) const {
  Entity::write(os);
  write_bin(os, z);
  write_bin(os, p1);
  write_bin(os, p2);
  write_bin(os, e);
  write_bin(os, f);
  write_bin(os, c1);
  write_bin(os, c2);
  write_bin(os, s2);
  write_bin(os, s3);
  write_bin(os, s4);
  write_bin(os, s5);
  os << '\n';
}

void Sides::read(std::istream &is) {
  Entity::read(is);
  read_bin(is, z);
  read_bin(is, p1);
  read_bin(is, p2);
  read_bin(is, e);
  read_bin(is, f);
  read_bin(is, c1);
  read_bin(is, c2);
  read_bin(is, s2);
  read_bin(is, s3);
  read_bin(is, s4);
  read_bin(is, s5);
  skip_line(is);
}

bool Sides::operator==(Sides const &rhs) const {
  return (Entity::operator==(rhs) && z == rhs.z && p1 == rhs.p1 &&
      p2 == rhs.p2 && e == rhs.e && f == rhs.f && c1 == rhs.c1 &&
      c2 == rhs.c2 && s2 == rhs.s2 && s3 == rhs.s3 && s4 == rhs.s4 &&
      s5 == rhs.s5);
}

void Sides::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  z.resize(total);
  p1.resize(total);
  p2.resize(total);
  e.resize(total);
  f.resize(total);
  c1.resize(total);
  c2.resize(total);
  s2.resize(total);
  s3.resize(total);
  s4.resize(total);
  s5.resize(total);
}

/* --------------------------------- Zones ----------------------------------*/

void Zones::write(std::ostream &os) const {
  Entity::write(os);
  os << '\n';
}

void Zones::read(std::istream &is) {
  Entity::read(is);
  skip_line(is);
}

bool Zones::operator==(Zones const &rhs) const {
  return (Entity::operator==(rhs));
}

void Zones::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
}

/* --------------------------------- Mesh -----------------------------------*/

std::ostream &operator<<(std::ostream &os, Mesh::Geometry_Type const &geo) {
  switch (geo) {
  case Mesh::CARTESIAN:
    os << "Cartesian";
    break;
  case Mesh::CYLINDRICAL:
    os << "Cylindrical";
    break;
  case Mesh::SPHERICAL:
    os << "Spherical";
    break;
  };
  return os;
}

void Mesh::write(std::ostream &os) const {
  write_bin(os, mype);
  write_bin(os, numpe);
  write_bin(os, geo);
  points.write(os);
  edges.write(os);
  faces.write(os);
  sides.write(os);
  corners.write(os);
  zones.write(os);
}

void Mesh::read(std::istream &is) {
  read_bin(is, mype);
  read_bin(is, numpe);
  read_bin(is, geo);
  points.read(is);
  edges.read(is);
  faces.read(is);
  sides.read(is);
  corners.read(is);
  zones.read(is);
}

bool Mesh::operator==(Mesh const &rhs) const {
  return mype == rhs.mype && numpe == rhs.numpe && geo == rhs.geo &&
      points == rhs.points && edges == rhs.edges && sides == rhs.sides &&
      corners == rhs.corners && zones == rhs.zones;
}

void Mesh::print_stats(std::ostream &os) const {
  os << "\tDecomposed Rank " << mype + 1 << "/" << numpe << '\n';
  os << "\tPoint dimensions: " << ndims() << '\n';
  os << "\tCoordinate system: " << geo << '\n';
  os << "\tPoints: " << points.size() << '\n';
  os << "\tZones: " << zones.size() << '\n';
  os << "\tSides: " << sides.size() << '\n';
  os << "\tEdges: " << edges.size() << '\n';
  os << "\tFaces: " << faces.size() << '\n';
  os << "\tCorners: " << corners.size() << '\n';
}

} // namespace SOA_Idx
} // namespace Ume
