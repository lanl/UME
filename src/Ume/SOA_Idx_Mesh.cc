/*!
  \file SOA_Idx_Mesh.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/utils.hh"
#include <istream>
#include <ostream>

namespace Ume {
namespace SOA_Idx {
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
  os << "\tDecomposed Rank " << mype << " (" << mype + 1 << "/" << numpe << ')'
     << '\n';
  os << "\tPoint dimensions: " << ndims() << '\n';
  os << "\tCoordinate system: " << geo << '\n';
  os << "\tPoints: " << points.lsize << '\n';
  os << "\tZones: " << zones.lsize << '\n';
  os << "\tSides: " << sides.lsize << '\n';
  os << "\tEdges: " << edges.lsize << '\n';
  os << "\tFaces: " << faces.lsize << '\n';
  os << "\tCorners: " << corners.lsize << '\n';
}

} // namespace SOA_Idx
} // namespace Ume
