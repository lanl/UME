/*!
  \file SOA_Idx_Mesh.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/soa_idx_geom.hh"
#include "Ume/soa_idx_helpers.hh"
#include <istream>
#include <ostream>

namespace Ume {
namespace SOA_Idx {
/* --------------------------------- Points ---------------------------------*/

Points::Points(Mesh *mesh) : Entity{mesh} {
  // point coordinates
  mesh_->ds->insert("pcoord", std::make_unique<Ume::DS_Entry>(Types::VEC3V));
}

void Points::write(std::ostream &os) const {
  Entity::write(os);
  write_bin(os, mesh_->ds->caccess_vec3v("pcoord"));
  os << '\n';
}

void Points::read(std::istream &is) {
  Entity::read(is);
  read_bin(is, mesh_->ds->access_vec3v("pcoord"));
  skip_line(is);
}

bool Points::operator==(Points const &rhs) const {
  return (Entity::operator==(rhs) &&
      mesh_->ds->caccess_vec3v("pcoord") ==
          rhs.mesh_->ds->caccess_vec3v("pcoord"));
}

void Points::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  (mesh_->ds->access_vec3v("pcoord")).resize(total);
}

/* --------------------------------- Mesh -----------------------------------*/

Mesh::Mesh()
    : ds{Datastore::create_root()}, corners{this}, edges{this}, faces{this},
      points{this}, sides{this}, zones{this} {}

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

// Define this here, where we have a complete definition of Mesh
Ume::Datastore *Entity::ds() { return mesh_->ds.get(); }
Ume::Datastore const *Entity::ds() const { return mesh_->ds.get(); }

} // namespace SOA_Idx
} // namespace Ume
