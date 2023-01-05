/*!
  \file SOA_Idx_Faces.cc
*/

#include "SOA_Idx_Mesh.hh"
#include "soa_idx_helpers.hh"

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Edges ----------------------------------*/

Edges::Edges(Mesh *mesh) : Entity{mesh} {
  // map: edge to end point 1 index
  mesh_->ds->insert("m:e>p1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: edge to end point 2 index
  mesh_->ds->insert("m:e>p2", std::make_unique<Ume::DS_Entry>(Types::INTV));
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

} // namespace SOA_Idx
} // namespace Ume
