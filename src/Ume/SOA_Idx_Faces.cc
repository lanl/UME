/*!
  \file SOA_Idx_Faces.cc
*/

#include "SOA_Idx_Mesh.hh"
#include "soa_idx_helpers.hh"

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Faces ----------------------------------*/

Faces::Faces(Mesh *mesh) : Entity{mesh} {
  // map: face to adjacent zone 1 index
  mesh_->ds->insert("m:f>z1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: face to adjacent zone 2 index
  mesh_->ds->insert("m:f>z2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  mesh_->ds->insert("fcoord", std::make_unique<DSE_fcoord>(*this));
}

void Faces::write(std::ostream &os) const {
  Entity::write(os);
  IVWRITE("m:f>z1");
  IVWRITE("m:f>z2");
  os << '\n';
}

void Faces::read(std::istream &is) {
  Entity::read(is);
  IVREAD("m:f>z1");
  IVREAD("m:f>z2");
  skip_line(is);
}

bool Faces::operator==(Faces const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:f>z1") && EQOP("m:f>z2"));
}

void Faces::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:f>z1", total);
  RESIZE("m:f>z2", total);
}

void Faces::DSE_fcoord::init_() const {
  DSE_INIT_PREAMBLE("DSE_fcoord");

  int const fll{faces_.size()};
  int const fl{faces_.lsize};
  int const sl{faces_.mesh_->sides.lsize};
  auto const &s2f{faces_.ds()->caccess_intv("m:s>f")};
  auto const &s2p1{faces_.ds()->caccess_intv("m:s>p1")};
  auto const &pcoord{faces_.ds()->caccess_vec3v("pcoord")};
  auto const &smask{faces_.mesh_->sides.mask};
  auto &fcoord = std::get<VEC3V_T>(data_);
  fcoord.resize(fll, Vec3(0.0));

  std::vector<int> num_face_pts(fl, 0);
  for (int s = 0; s < sl; ++s) {
    if (smask[s]) {
      int const f = s2f[s];
      fcoord[f] += pcoord[s2p1[s]];
      num_face_pts.at(f) += 1;
    }
  }

  auto const &fmask{faces_.mask};
  for (int f = 0; f < fl; ++f) {
    if (fmask[f]) {
      fcoord[f] /= static_cast<double>(num_face_pts[f]);
    }
  }

  init_state_ = Init_State::INITIALIZED;
}
} // namespace SOA_Idx
} // namespace Ume
