/*!
  \file SOA_Idx_Points.cc
*/

#include "SOA_Idx_Mesh.hh"
#include "soa_idx_helpers.hh"

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

} // namespace SOA_Idx
} // namespace Ume
