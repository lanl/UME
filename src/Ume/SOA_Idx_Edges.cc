/*!
  \file Ume/SOA_Idx_Faces.cc
*/

#include "SOA_Idx_Mesh.hh"
#include "soa_idx_helpers.hh"
#include <cassert>

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Edges ----------------------------------*/

Edges::Edges(Mesh *mesh) : Entity{mesh} {
  // map: edge to end point 1 index
  ds().insert("m:e>p1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: edge to end point 2 index
  ds().insert("m:e>p2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("ecoord", std::make_unique<DSE_ecoord>(*this));
}

void Edges::write(std::ostream &os) const {
  write_bin(os, std::string("edges"));
  Entity::write(os);
  IVWRITE("m:e>p1");
  IVWRITE("m:e>p2");
}

void Edges::read(std::istream &is) {
  std::string dummy;
  read_bin(is, dummy);
  assert(dummy == "edges");
  Entity::read(is);
  IVREAD("m:e>p1");
  IVREAD("m:e>p2");
}

bool Edges::operator==(Edges const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:e>p1") && EQOP("m:e>p2"));
}

void Edges::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:e>p1", total);
  RESIZE("m:e>p2", total);
}

bool Edges::DSE_ecoord::init_() const {
  DSE_INIT_PREAMBLE("DSE_ecoord");

  int const el = edges().lsize;
  int const ell = edges().size();
  auto const &e2p1{caccess_intv("m:e>p1")};
  auto const &e2p2{caccess_intv("m:e>p2")};
  auto const &pcoord{caccess_vec3v("pcoord")};
  auto const &emask{edges().mask};
  auto &ecoord = mydata_vec3v();
  ecoord.resize(ell);

  for (int e = 0; e < el; ++e) {
    if (emask[e]) {
      ecoord[e] = (pcoord[e2p1[e]] + pcoord[e2p2[e]]) * 0.5;
    } else {
      ecoord[e] = 0.0;
    }
  }

  DSE_INIT_EPILOGUE;
}

} // namespace SOA_Idx
} // namespace Ume
