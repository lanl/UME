/*!
  \file SOA_Idx_Mesh.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/utils.hh"
#include <istream>
#include <ostream>

#define EQOP(N) (ds_->caccess_intv(N) == rhs.ds_->caccess_intv(N))
#define IVWRITE(N) write_bin(os, ds_->caccess_intv(N))
#define IVREAD(N) read_bin(is, ds_->access_intv(N))
#define RESIZE(N, S) (ds_->access_intv(N)).resize(S)

namespace Ume {
namespace SOA_Idx {
/* -------------------------------- Corners ---------------------------------*/

Corners::Corners(dsptr ds) : Entity{ds} {
  // map: corner->index of characteristic point
  ds_->insert("m:c>p", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: corner->index of parent zone zone
  ds_->insert("m:c>z", std::make_unique<Ume::DS_Entry>(Types::INTV));
}

void Corners::write(std::ostream &os) const {
  Entity::write(os);
  IVWRITE("m:c>p");
  IVWRITE("m:c>z");
  os << '\n';
}

void Corners::read(std::istream &is) {
  Entity::read(is);
  IVREAD("m:c>p");
  IVREAD("m:c>z");
  skip_line(is);
}

bool Corners::operator==(Corners const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:c>p") && EQOP("m:c>z"));
}

void Corners::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:c>p", total);
  RESIZE("m:c>z", total);
}

/* --------------------------------- Edges ----------------------------------*/

Edges::Edges(dsptr ds) : Entity{ds} {
  // map: edge to end point 1 index
  ds_->insert("m:e>p1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: edge to end point 2 index
  ds_->insert("m:e>p2", std::make_unique<Ume::DS_Entry>(Types::INTV));
}

void Edges::write(std::ostream &os) const {
  Entity::write(os);
  IVWRITE("m:e>p1");
  IVWRITE("m:e>p2");
  os << '\n';
}

void Edges::read(std::istream &is) {
  Entity::read(is);
  IVREAD("m:e>p1");
  IVREAD("m:e>p2");
  skip_line(is);
}

bool Edges::operator==(Edges const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:e>p1") && EQOP("m:e>p2"));
}

void Edges::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:e>p1", total);
  RESIZE("m:e>p2", total);
}

/* --------------------------------- Faces ----------------------------------*/

Faces::Faces(dsptr ds) : Entity{ds} {
  // map: face to adjacent zone 1 index
  ds_->insert("m:f>z1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: face to adjacent zone 2 index
  ds_->insert("m:f>z2", std::make_unique<Ume::DS_Entry>(Types::INTV));
}

void Faces::write(std::ostream &os) const {
  Entity::write(os);
  IVWRITE("m:f>z1");
  IVWRITE("m:f>z2");
  os << '\n';
}

void Faces::read(std::istream &is) {
  Entity::read(is);
  IVREAD("m:f>z1");
  IVREAD("m:f>z2");
  skip_line(is);
}

bool Faces::operator==(Faces const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:f>z1") && EQOP("m:f>z2"));
}

void Faces::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:f>z1", total);
  RESIZE("m:f>z2", total);
}

/* --------------------------------- Points ---------------------------------*/

Points::Points(dsptr ds) : Entity{ds} {
  // point coordinates
  ds_->insert("pcoord", std::make_unique<Ume::DS_Entry>(Types::VEC3V));
}

void Points::write(std::ostream &os) const {
  Entity::write(os);
  write_bin(os, ds_->caccess_vec3v("pcoord"));
  os << '\n';
}

void Points::read(std::istream &is) {
  Entity::read(is);
  read_bin(is, ds_->access_vec3v("pcoord"));
  skip_line(is);
}

bool Points::operator==(Points const &rhs) const {
  return (Entity::operator==(rhs) &&
      ds_->caccess_vec3v("pcoord") == rhs.ds_->caccess_vec3v("pcoord"));
}

void Points::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  (ds_->access_vec3v("pcoord")).resize(total);
}

/* --------------------------------- Sides ----------------------------------*/

Sides::Sides(dsptr ds) : Entity{ds} {
  // Index of parent mesh zone
  ds->insert("m:s>z", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Index of characteristic edge
  ds->insert("m:s>e", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Indices of the points of 'e' (redundant, but heavily used)
  ds->insert("m:s>p1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds->insert("m:s>p2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Index of the mesh face of z that contains e
  ds->insert("m:s>f", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Indices of the corners of z that this side intersects
  ds->insert("m:s>c1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds->insert("m:s>c2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  /* The indices of the sides adjacent to this one.  Note that one of these
     will belong to another zone. */
  ds->insert("m:s>s2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds->insert("m:s>s3", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds->insert("m:s>s4", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds->insert("m:s>s5", std::make_unique<Ume::DS_Entry>(Types::INTV));
}

void Sides::write(std::ostream &os) const {
  Entity::write(os);
  IVWRITE("m:s>z");
  IVWRITE("m:s>p1");
  IVWRITE("m:s>p2");
  IVWRITE("m:s>e");
  IVWRITE("m:s>f");
  IVWRITE("m:s>c1");
  IVWRITE("m:s>c2");
  IVWRITE("m:s>s2");
  IVWRITE("m:s>s3");
  IVWRITE("m:s>s4");
  IVWRITE("m:s>s5");
  os << '\n';
}

void Sides::read(std::istream &is) {
  Entity::read(is);
  IVREAD("m:s>z");
  IVREAD("m:s>p1");
  IVREAD("m:s>p2");
  IVREAD("m:s>e");
  IVREAD("m:s>f");
  IVREAD("m:s>c1");
  IVREAD("m:s>c2");
  IVREAD("m:s>s2");
  IVREAD("m:s>s3");
  IVREAD("m:s>s4");
  IVREAD("m:s>s5");
  skip_line(is);
}

bool Sides::operator==(Sides const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:s>z") && EQOP("m:s>p1") &&
      EQOP("m:s>p2") && EQOP("m:s>e") && EQOP("m:s>f") && EQOP("m:s>c1") &&
      EQOP("m:s>c2") && EQOP("m:s>s2") && EQOP("m:s>s3") && EQOP("m:s>s4") &&
      EQOP("m:s>s5"));
}

void Sides::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:s>z", total);
  RESIZE("m:s>p1", total);
  RESIZE("m:s>p2", total);
  RESIZE("m:s>e", total);
  RESIZE("m:s>f", total);
  RESIZE("m:s>c1", total);
  RESIZE("m:s>c2", total);
  RESIZE("m:s>s2", total);
  RESIZE("m:s>s3", total);
  RESIZE("m:s>s4", total);
  RESIZE("m:s>s5", total);
}

/* --------------------------------- Zones ----------------------------------*/

Zones::Zones(dsptr ds) : Entity{ds} {}

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

Mesh::Mesh()
    : ds{Datastore::create_root()}, corners{ds}, edges{ds}, faces{ds},
      points{ds}, sides{ds}, zones{ds} {}

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
  os << "\tZones: " << zones.lsize << ' ' << zones.size() << '\n';
  os << "\tSides: " << sides.lsize << ' ' << sides.size() << '\n';
  os << "\tEdges: " << edges.lsize << '\n';
  os << "\tFaces: " << faces.lsize << '\n';
  os << "\tCorners: " << corners.lsize << ' ' << corners.size() << '\n';
}

} // namespace SOA_Idx
} // namespace Ume
