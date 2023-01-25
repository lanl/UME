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
  ds().insert("pcoord", std::make_unique<Ume::DS_Entry>(Types::VEC3V));
  ds().insert("point_norm", std::make_unique<DSE_point_norm>(*this));
}

void Points::write(std::ostream &os) const {
  Entity::write(os);
  write_bin(os, ds().caccess_vec3v("pcoord"));
  os << '\n';
}

void Points::read(std::istream &is) {
  Entity::read(is);
  read_bin(is, ds().access_vec3v("pcoord"));
  skip_line(is);
}

bool Points::operator==(Points const &rhs) const {
  return (Entity::operator==(rhs) &&
      ds().caccess_vec3v("pcoord") == rhs.ds().caccess_vec3v("pcoord"));
}

void Points::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  (ds().access_vec3v("pcoord")).resize(total);
}

bool Points::DSE_point_norm::init_() const {
  DSE_INIT_PREAMBLE("DSE_point_norm");
  int const pll = points().size();
  int const pl = points().lsize;
  int const sl = sides().lsize;
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
  // points().gathscat(Comm::Op::SUM, point_norm);
  for (int p = 0; p < pl; ++p) {
    if (pmask[p] < 0) {
      normalize(point_norm[p]);
    }
  }
  DSE_INIT_EPILOGUE;
}

} // namespace SOA_Idx
} // namespace Ume
