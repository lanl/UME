/*!
  \file SOA_Idx_Zones.hh
*/

#include "SOA_Idx_Mesh.hh"
#include "soa_idx_helpers.hh"

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Zones ----------------------------------*/

Zones::Zones(Mesh *mesh) : Entity{mesh} {
  ds().insert("zcoord", std::make_unique<DSE_zcoord>(*this));
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

bool Zones::DSE_zcoord::init_() const {
  DSE_INIT_PREAMBLE("DSE_zcoord");

  int const zl = zones().lsize;
  int const zll = zones().size();
  int const cl = corners().lsize;
  auto const &c2z{caccess_intv("m:c>z")};
  auto const &c2p{caccess_intv("m:c>p")};
  auto const &pcoord{caccess_vec3v("pcoord")};
  auto const &cmask{corners().mask};

  auto &zcoord = mydata_vec3v();
  zcoord.resize(zll, Vec3(0.0));

  std::vector<int> num_zone_pts(zl, 0);
  for (int c = 0; c < cl; ++c) {
    if (cmask[c]) {
      int const z = c2z[c];
      zcoord[z] += pcoord[c2p[c]];
      num_zone_pts.at(z) += 1;
    }
  }

  auto const &zmask{zones().mask};
  for (int z = 0; z < zl; ++z) {
    if (zmask[z]) {
      zcoord[z] /= static_cast<double>(num_zone_pts[z]);
    }
  }
  DSE_INIT_EPILOGUE;
}
} // namespace SOA_Idx
} // namespace Ume
