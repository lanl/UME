/*!
  \file Ume/SOA_Idx_Zones.hh
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/soa_idx_helpers.hh"
#include <set>

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Zones ----------------------------------*/

Zones::Zones(Mesh *mesh) : Entity{mesh} {
  ds().insert("zcoord", std::make_unique<VAR_zcoord>(*this));
  ds().insert("m:z>pz", std::make_unique<VAR_zone_to_pt_zone>(*this));
  ds().insert("m:z>p", std::make_unique<VAR_zone_to_points>(*this));
}

void Zones::write(std::ostream &os) const {
  write_bin(os, std::string{"zones"});
  Entity::write(os);
}

void Zones::read(std::istream &is) {
  std::string dummy;
  read_bin(is, dummy);
  assert(dummy == "zones");
  Entity::read(is);
}

bool Zones::operator==(Zones const &rhs) const {
  return (Entity::operator==(rhs));
}

void Zones::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
}

bool Zones::VAR_zcoord::init_() const {
  VAR_INIT_PREAMBLE("VAR_zcoord");

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
  zones().scatter(zcoord);
  VAR_INIT_EPILOGUE;
}

bool Zones::VAR_zone_to_pt_zone::init_() const {
  VAR_INIT_PREAMBLE("VAR_zone_to_pt_zone");
  int const pll = points().size();
  int const zll = zones().size();
  int const cll = corners().size();
  auto const &p2zs{caccess_intrr("m:p>zs")};
  auto const &c2p{caccess_intv("m:c>p")};
  auto const &c2z{caccess_intv("m:c>z")};
  auto &z2pz = mydata_intrr();
  z2pz.init(zll);
  std::vector<std::set<int>> accum(zll);

  /* Iterate over corners, add all zones attached to c2p[c] to c2z[z]; */
  for (int c = 0; c < cll; ++c) {
    int const p = c2p[c];
    int const z = c2z[c];
    if (p < pll && z < zll)
      accum.at(z).insert(p2zs[p].begin(), p2zs[p].end());
  }

  /* Fill the ragged-right arrays, eliminating zone self-links */
  for (int z = 0; z < zll; ++z) {
    accum[z].erase(z);
    z2pz.assign(z, accum[z].begin(), accum[z].end());
  }
  VAR_INIT_EPILOGUE;
}

bool Zones::VAR_zone_to_points::init_() const {
  VAR_INIT_PREAMBLE("VAR_zone_to_points");
  int const pll = points().size();
  int const zll = zones().size();
  int const cll = corners().size();
  auto const &c2p{caccess_intv("m:c>p")};
  auto const &c2z{caccess_intv("m:c>z")};
  auto &z2p = mydata_intrr();
  z2p.init(zll);
  std::vector<std::vector<int>> accum(zll);

  /* Iterate over corners, connect points to zones */
  for (int c = 0; c < cll; ++c) {
    int const p = c2p[c];
    int const z = c2z[c];
    if (p < pll && z < zll)
      accum.at(z).push_back(p);
  }

  /* Fill the ragged-right arrays*/
  for (int z = 0; z < zll; ++z) {
    std::sort(accum[z].begin(), accum[z].end());
    z2p.assign(z, accum[z].begin(), accum[z].end());
  }
  VAR_INIT_EPILOGUE;
}

} // namespace SOA_Idx
} // namespace Ume
