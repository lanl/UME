/*!
  \file SOA_Idx_Sides.hh
*/

#include "SOA_Idx_Mesh.hh"
#include "soa_idx_helpers.hh"

namespace Ume {
namespace SOA_Idx {

Sides::Sides(Mesh *mesh) : Entity{mesh} {
  // Index of parent mesh zone
  mesh_->ds->insert("m:s>z", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Index of characteristic edge
  mesh_->ds->insert("m:s>e", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Indices of the points of 'e' (redundant, but heavily used)
  mesh_->ds->insert("m:s>p1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  mesh_->ds->insert("m:s>p2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Index of the mesh face of z that contains e
  mesh_->ds->insert("m:s>f", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Indices of the corners of z that this side intersects
  mesh_->ds->insert("m:s>c1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  mesh_->ds->insert("m:s>c2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  /* The indices of the sides adjacent to this one.  Note that one of these
     will belong to another zone. */
  mesh_->ds->insert("m:s>s2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  mesh_->ds->insert("m:s>s3", std::make_unique<Ume::DS_Entry>(Types::INTV));
  mesh_->ds->insert("m:s>s4", std::make_unique<Ume::DS_Entry>(Types::INTV));
  mesh_->ds->insert("m:s>s5", std::make_unique<Ume::DS_Entry>(Types::INTV));
  mesh_->ds->insert(
      "side_area_norm", std::make_unique<DSE_side_area_norm>(*this));
  mesh_->ds->insert("side_vol", std::make_unique<DSE_side_vol>(*this));
}

void Sides::write(std::ostream &os) const {
  Entity::write(os);
  IVWRITE("m:s>z");
  IVWRITE("m:s>p1");
  IVWRITE("m:s>p2");
  IVWRITE("m:s>e");
  IVWRITE("m:s>f");
  IVWRITE("m:s>c1");
  IVWRITE("m:s>c2");
  IVWRITE("m:s>s2");
  IVWRITE("m:s>s3");
  IVWRITE("m:s>s4");
  IVWRITE("m:s>s5");
  os << '\n';
}

void Sides::read(std::istream &is) {
  Entity::read(is);
  IVREAD("m:s>z");
  IVREAD("m:s>p1");
  IVREAD("m:s>p2");
  IVREAD("m:s>e");
  IVREAD("m:s>f");
  IVREAD("m:s>c1");
  IVREAD("m:s>c2");
  IVREAD("m:s>s2");
  IVREAD("m:s>s3");
  IVREAD("m:s>s4");
  IVREAD("m:s>s5");
  skip_line(is);
}

bool Sides::operator==(Sides const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:s>z") && EQOP("m:s>p1") &&
      EQOP("m:s>p2") && EQOP("m:s>e") && EQOP("m:s>f") && EQOP("m:s>c1") &&
      EQOP("m:s>c2") && EQOP("m:s>s2") && EQOP("m:s>s3") && EQOP("m:s>s4") &&
      EQOP("m:s>s5"));
}

void Sides::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:s>z", total);
  RESIZE("m:s>p1", total);
  RESIZE("m:s>p2", total);
  RESIZE("m:s>e", total);
  RESIZE("m:s>f", total);
  RESIZE("m:s>c1", total);
  RESIZE("m:s>c2", total);
  RESIZE("m:s>s2", total);
  RESIZE("m:s>s3", total);
  RESIZE("m:s>s4", total);
  RESIZE("m:s>s5", total);
}

void Sides::DSE_side_area_norm::init_() const {
  DSE_INIT_PREAMBLE("DSE_side_vol");
  int const sl = sides().lsize;
  int const sll = sides().size();

  auto const &s2p1 = caccess_intv("m:s>p1");
  auto const &s2p2 = caccess_intv("m:s>p2");
  auto const &s2e = caccess_intv("m:s>e");
  auto const &s2f = caccess_intv("m:s>f");
  auto const &s2z = caccess_intv("m:s>z");

  auto const &ex = caccess_vec3v("ecoord");
  auto const &fx = caccess_vec3v("fcoord");
  auto const &px = caccess_vec3v("pcoord");
  auto const &zx = caccess_vec3v("zcoord");

  auto const &smask{sides().mask};
  auto &side_area_norm = mydata_vec3v();
  side_area_norm.resize(sll);

  for (int s = 0; s < sl; ++s) {
    if (smask[s] > 0) {
      // A (non-ghost) side in the interior of the mesh
      Vec3 const &zc = zx[s2z[s]];
      Vec3 const &ep = ex[s2e[s]];
      Vec3 const &fp = fx[s2f[s]];
      side_area_norm[s] = crossprod(ep - zc, fp - zc) / 2.0;
    } else if (smask[s] < 0) {
      // A side on a mesh boundary face
      Vec3 const &fc = fx[s2f[s]];
      Vec3 const &p1 = px[s2p1[s]];
      Vec3 const &p2 = px[s2p2[s]];
      side_area_norm[s] = crossprod(p1 - fc, p2 - fc) / 4.0; // Deliberate
    } else
      side_area_norm[s] = 0.0;
  }
  init_state_ = Init_State::INITIALIZED;
}

void Sides::DSE_side_vol::init_() const {
  DSE_INIT_PREAMBLE("DSE_side_vol");
  int const sl = sides().lsize;
  int const sll = sides().size();
  auto const &s2z = caccess_intv("m:s>z");
  auto const &s2p1 = caccess_intv("m:s>p1");
  auto const &s2p2 = caccess_intv("m:s>p2");
  auto const &s2f = caccess_intv("m:s>f");
  auto const &px = caccess_vec3v("pcoord");
  auto const &zx = caccess_vec3v("zcoord");
  auto const &fx = caccess_vec3v("fcoord");
  auto const &smask{sides().mask};
  auto &side_vol = mydata_dblv();
  side_vol.resize(sll);

  for (int s = 0; s < sl; ++s) {
    if (smask[s] > 0) {
      Vec3 const &zc = zx[s2z[s]];
      Vec3 const &p1 = px[s2p2[s]];
      Vec3 const &p2 = px[s2p1[s]];
      Vec3 const &fc = fx[s2f[s]];
      /* Note that this is a signed volume of the tetrahedron formed by the zone
         center, face center, and edge endpoints. */
      side_vol[s] = dotprod(fc - zc, crossprod(p1 - zc, p2 - zc)) / 6.0;
    } else
      side_vol[s] = 0.0;
  }
  init_state_ = Init_State::INITIALIZED;
}

} // namespace SOA_Idx
} // namespace Ume
