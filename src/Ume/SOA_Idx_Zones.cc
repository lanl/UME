/*!
  \file SOA_Idx_Zones.hh
*/

#include "SOA_Idx_Mesh.hh"
#include "soa_idx_helpers.hh"

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Zones ----------------------------------*/

Zones::Zones(Mesh *mesh) : Entity{mesh} {
  mesh_->ds->insert("zcoord", std::make_unique<DSE_zcoord>(*this));
}

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

void Zones::DSE_zcoord::init_() const {
  DSE_INIT_PREAMBLE("DSE_zcoord");

  int const zl{zones_.lsize};
  int const cl{zones_.mesh_->corners.lsize};
  auto const &c2z{zones_.ds()->caccess_intv("m:c>z")};
  auto const &c2p{zones_.ds()->caccess_intv("m:c>p")};
  auto const &pcoord{zones_.ds()->caccess_vec3v("pcoord")};
  auto const &cmask{zones_.mesh_->corners.mask};

  auto &zcoord = std::get<VEC3V_T>(data_);
  zcoord.resize(zl, Vec3(0.0));

  std::vector<int> num_zone_pts(zl, 0);
  for (int c = 0; c < cl; ++c) {
    if (cmask[c]) {
      int const z = c2z[c];
      zcoord[z] += pcoord[c2p[c]];
      num_zone_pts.at(z) += 1;
    }
  }

  auto const &zmask{zones_.mask};
  for (int z = 0; z < zl; ++z) {
    if (zmask[z]) {
      zcoord[z] /= static_cast<double>(num_zone_pts[z]);
    }
  }

  init_state_ = Init_State::INITIALIZED;
}
} // namespace SOA_Idx
} // namespace Ume
