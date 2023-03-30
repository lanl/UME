/*!
  \file Ume/SOA_Idx_Points.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/soa_idx_helpers.hh"
#include <cassert>
#include <iostream>

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Points ---------------------------------*/

Points::Points(Mesh *mesh) : Entity{mesh} {
  // point coordinates
  ds().insert("pcoord", std::make_unique<Ume::DS_Entry>(Types::VEC3V));
  ds().insert("point_norm", std::make_unique<VAR_point_norm>(*this));
  ds().insert("m:p>zs", std::make_unique<VAR_point_to_zones>(*this));
  ds().insert("m:p>rc", std::make_unique<VAR_point_to_real_corners>(*this));
}

void Points::write(std::ostream &os) const {
  write_bin(os, std::string("points"));
  Entity::write(os);
  write_bin(os, ds().caccess_vec3v("pcoord"));
}

void Points::read(std::istream &is) {
  std::string dummy;
  read_bin(is, dummy);
  assert(dummy == "points");
  Entity::read(is);
  read_bin(is, ds().access_vec3v("pcoord"));
}

bool Points::operator==(Points const &rhs) const {
  return (Entity::operator==(rhs) &&
      ds().caccess_vec3v("pcoord") == rhs.ds().caccess_vec3v("pcoord"));
}

void Points::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  (ds().access_vec3v("pcoord")).resize(total);
}

bool Points::VAR_point_to_zones::init_() const {
  VAR_INIT_PREAMBLE("VAR_point_to_zones");
  int const pll = points().size();
  int const cll = corners().size();
  int const zll = zones().size();
  auto const &c2z{caccess_intv("m:c>z")};
  auto const &c2p{caccess_intv("m:c>p")};
  auto &p2zs = mydata_intrr();

  p2zs.init(pll);
  std::vector<std::vector<int>> accum(pll);
  for (int c = 0; c < cll; ++c) {
    int const z = c2z[c];
    int const p = c2p[c];
    if (p < pll && z < zll) // Some c2z values are bad at ghost corners
      accum.at(p).push_back(z);
  }
  for (int p = 0; p < pll; ++p) {
    std::sort(accum[p].begin(), accum[p].end());
    p2zs.assign(p, accum[p].begin(), accum[p].end());
  }

  VAR_INIT_EPILOGUE;
}

bool Points::VAR_point_norm::init_() const {
  VAR_INIT_PREAMBLE("VAR_point_norm");
  int const pll = points().size();
  int const pl = points().local_size();
  int const sl = sides().local_size();
  auto const &s2s2{caccess_intv("m:s>s2")};
  auto const &s2p1{caccess_intv("m:s>p1")};
  auto const &s2p2{caccess_intv("m:s>p2")};
  auto const &side_surz{caccess_vec3v("side_surz")};
  auto const &smask{sides().mask};
  auto const &pmask{points().mask};
  auto &point_norm = mydata_vec3v();

  point_norm.resize(pll, Vec3(0.0));

  for (int s = 0; s < sl; ++s) {
    if (smask[s] == -1) { // boundary side (outside of real mesh)
      int const s2 = s2s2[s]; // the corresponding real side
      int const p1 = s2p1[s2];
      int const p2 = s2p2[s2];
      point_norm[p1] += side_surz[s2];
      point_norm[p2] += side_surz[s2];
    }
  }

  /* Since points are shared among adjacent parallel ranks, we need to do a
     parallel sum. */
  points().gathscat(Comm::Op::SUM, point_norm);
  for (int p = 0; p < pl; ++p) {
    if (pmask[p] < 0) {
      normalize(point_norm[p]);
    }
  }
  VAR_INIT_EPILOGUE;
}

bool Points::VAR_point_to_real_corners::init_() const {
  VAR_INIT_PREAMBLE("VAR_point_to_zones");
  int const pll = points().size();
  int const cl = corners().local_size();
  auto const &c2p{caccess_intv("m:c>p")};
  auto const &cmask{corners().mask};
  auto &p2rc = mydata_intrr();

  p2rc.init(pll);
  std::vector<std::vector<int>> accum(pll);
  for (int c = 0; c < cl; ++c) {
    if (cmask[c] < 1)
      continue; // only take non-ghost/non-boundary corners
    int const p = c2p[c];
    accum.at(p).push_back(c);
  }
  for (int p = 0; p < pll; ++p) {
    p2rc.assign(p, accum[p].begin(), accum[p].end());
  }
  VAR_INIT_EPILOGUE;
}

} // namespace SOA_Idx
} // namespace Ume
