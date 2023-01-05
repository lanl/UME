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
  mesh_->ds->insert("ecoord", std::make_unique<DSE_ecoord>(*this));
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

void Edges::DSE_ecoord::init_() const {
  DSE_INIT_PREAMBLE("DSE_ecoord");

  int const el{edges_.lsize};
  int const ell{edges_.size()};
  auto const &e2p1{edges_.ds()->caccess_intv("m:e>p1")};
  auto const &e2p2{edges_.ds()->caccess_intv("m:e>p2")};
  auto const &pcoord{edges_.ds()->caccess_vec3v("pcoord")};
  auto const &emask{edges_.mesh_->edges.mask};
  auto &ecoord = std::get<VEC3V_T>(data_);
  ecoord.resize(ell, Vec3(0.0));

  for (int e = 0; e < el; ++e) {
    if (emask[e]) {
      ecoord[e] = (pcoord[e2p1[e]] + pcoord[e2p2[e]]) * 0.5;
    }
  }

  init_state_ = Init_State::INITIALIZED;
}

} // namespace SOA_Idx
} // namespace Ume
