/*!
  \file SOA_Idx_Faces.cc
*/

#include "SOA_Idx_Mesh.hh"
#include "soa_idx_helpers.hh"

namespace Ume {
namespace SOA_Idx {

/* -------------------------------- Corners ---------------------------------*/

Corners::Corners(Mesh *mesh) : Entity{mesh} {
  // map: corner->index of characteristic point
  mesh_->ds->insert("m:c>p", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: corner->index of parent zone zone
  mesh_->ds->insert("m:c>z", std::make_unique<Ume::DS_Entry>(Types::INTV));
  mesh_->ds->insert("corner_vol", std::make_unique<DSE_corner_vol>(*this));
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

void Corners::DSE_corner_vol::init_() const {
  DSE_INIT_PREAMBLE("DSE_corner_vol");

  int const cll = corners().size();
  int const sl = sides().lsize;
  auto const &s2c1{caccess_intv("m:s>c1")};
  auto const &s2c2{caccess_intv("m:s>c2")};
  auto const &side_vol{caccess_dblv("side_vol")};
  auto const &smask{sides().mask};
  auto &corner_vol = mydata_dblv();
  corner_vol.resize(cll, 0.0);

  for (int s = 0; s < sl; ++s) {
    if (smask[s] > 0) {
      double const hsv = 0.5 * side_vol[s];
      corner_vol[s2c1[s]] += hsv;
      corner_vol[s2c2[s]] += hsv;
    }
  }
  init_state_ = Init_State::INITIALIZED;
}
} // namespace SOA_Idx
} // namespace Ume
