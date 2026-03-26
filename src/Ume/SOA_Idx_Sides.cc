/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*!
  \file Ume/SOA_Idx_Sides.hh
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/soa_idx_helpers.hh"
#include "Ume/mem_exec_spaces.hh"
#include <cassert>

namespace Ume {
namespace SOA_Idx {

Sides::Sides(Mesh *mesh) : Entity{mesh} {
  // Index of parent mesh zone
  ds().insert("m:s>z", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Index of characteristic edge
  ds().insert("m:s>e", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Indices of the points of 'e' (redundant, but heavily used)
  ds().insert("m:s>p1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("m:s>p2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Index of the mesh face of z that contains e
  ds().insert("m:s>f", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Indices of the corners of z that this side intersects
  ds().insert("m:s>c1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("m:s>c2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  /* The indices of the sides adjacent to this one.  Note that one of these
     will belong to another zone. */
  ds().insert("m:s>s2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("m:s>s3", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("m:s>s4", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("m:s>s5", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("side_surf", std::make_unique<VAR_side_surf>(*this));
  ds().insert("side_surz", std::make_unique<VAR_side_surz>(*this));
  ds().insert("side_vol", std::make_unique<VAR_side_vol>(*this));
}

void Sides::write(std::ostream &os) const {
  write_bin(os, std::string{"sides"});
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
}

void Sides::read(std::istream &is) {
  std::string dummy;
  read_bin(is, dummy);
  assert(dummy == "sides");
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

bool Sides::VAR_side_surf::init_() const {
  VAR_INIT_PREAMBLE("VAR_side_surf");
  int const sl = sides().local_size();
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
  auto &side_surf = mydata_vec3v();
  side_surf.assign(sll, VEC3_T(0.0));

  Kokkos::View<Vec3 *, HostSpace> h_side_surf(&side_surf[0], sl);
  Kokkos::View<const Vec3 *, HostSpace> h_ex(&ex[0], sl);
  Kokkos::View<const Vec3 *, HostSpace> h_fx(&fx[0], sl);
  Kokkos::View<const Vec3 *, HostSpace> h_px(&px[0], sl);
  Kokkos::View<const Vec3 *, HostSpace> h_zx(&zx[0], sl);
  Kokkos::View<const int *, HostSpace> h_s2p1(&s2p1[0], sl);
  Kokkos::View<const int *, HostSpace> h_s2p2(&s2p2[0], sl);
  Kokkos::View<const int *, HostSpace> h_s2e(&s2e[0], sl);
  Kokkos::View<const int *, HostSpace> h_s2f(&s2f[0], sl);
  Kokkos::View<const int *, HostSpace> h_s2z(&s2z[0], sl);
  Kokkos::View<const short *, HostSpace> h_smask(
      &smask[0], smask.size());

  Kokkos::parallel_for(
      "VAR_side_surf", Kokkos::RangePolicy<HostExecSpace>(0, sl), [&](const int s) {
        if (h_smask(s) > 0) {
          // A real side in the interior of the mesh
          Vec3 const &zc = h_zx(h_s2z(s));
          Vec3 const &ep = h_ex(h_s2e(s));
          Vec3 const &fp = h_fx(h_s2f(s));
          //    Area-weighted normal of triangle <ep, fp, zc>.  The corners that
          //   intersect this side share a face in the plane of that triangle.
          // auto tmp =
          h_side_surf(s) = crossprod(ep - zc, fp - zc) / 2.0;
        } else if (h_smask(s) < 0) {
          /* A ghost side on a mesh boundary face.  There isn't really a zx
             here, so we compute it differently */
          Vec3 const &fc = h_fx(h_s2f(s));
          Vec3 const &p1 = h_px(h_s2p1(s));
          Vec3 const &p2 = h_px(h_s2p2(s));
          h_side_surf(s) = crossprod(p1 - fc, p2 - fc) / 4.0; // Deliberate
        } else
          h_side_surf(s) = 0.0;
      });

  sides().scatter(side_surf);
  VAR_INIT_EPILOGUE;
}

bool Sides::VAR_side_surz::init_() const {
  VAR_INIT_PREAMBLE("VAR_side_surz");
  int const sl = sides().local_size();
  int const sll = sides().size();

  auto const &s2p1 = caccess_intv("m:s>p1");
  auto const &s2p2 = caccess_intv("m:s>p2");
  auto const &s2f = caccess_intv("m:s>f");

  auto const &fx = caccess_vec3v("fcoord");
  auto const &px = caccess_vec3v("pcoord");

  auto const &smask{sides().mask};
  auto &side_surz = mydata_vec3v();
  side_surz.assign(sll, VEC3_T(0.0)); //

  Kokkos::View<Vec3 *, HostSpace> h_side_surz_k(&side_surz[0], sl);
  Kokkos::View<const Vec3 *, HostSpace> h_fx(&fx[0], sl);
  Kokkos::View<const Vec3 *, HostSpace> h_px(&px[0], sl);
  Kokkos::View<const int *, HostSpace> h_s2p1(&s2p1[0], sl);
  Kokkos::View<const int *, HostSpace> h_s2p2(&s2p2[0], sl);
  Kokkos::View<const int *, HostSpace> h_s2f(&s2f[0], sl);
  Kokkos::View<const short *, HostSpace> h_smask(
      &smask[0], smask.size());

  Kokkos::parallel_for(
      "VAR_side_surz", Kokkos::RangePolicy<HostExecSpace>(0, sl), [&](const int s) {
        if (h_smask(s)) {
          // A non-ghost side
          Vec3 const &fc = h_fx(h_s2f(s)); // 0
          Vec3 const &p1 = h_px(h_s2p1(s)); // 2
          Vec3 const &p2 = h_px(h_s2p2(s)); // 1
          // Area-weighted normal of triangle <p2, p1, fc>
          h_side_surz_k(s) = crossprod(p2 - fc, p1 - fc) / 2.0;
        } else {
          h_side_surz_k(s) = 0.0;
        }
      });

  sides().scatter(side_surz);
  VAR_INIT_EPILOGUE;
}

bool Sides::VAR_side_vol::init_() const {
  VAR_INIT_PREAMBLE("VAR_side_vol");
  int const sl = sides().local_size();
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
  side_vol.assign(sll, 0.0);

  Kokkos::View<double *, HostSpace> h_side_vol_k(&side_vol[0], sl);
  Kokkos::View<const Vec3 *, HostSpace> h_px(&px[0], px.size());
  Kokkos::View<const Vec3 *, HostSpace> h_zx(&zx[0], zx.size());
  Kokkos::View<const Vec3 *, HostSpace> h_fx(&fx[0], fx.size());
  Kokkos::View<const int *, HostSpace> h_s2p1(s2p1.data(), s2p1.size());
  Kokkos::View<const int *, HostSpace> h_s2p2(s2p2.data(), s2p2.size());
  Kokkos::View<const int *, HostSpace> h_s2z(s2z.data(), s2z.size());
  Kokkos::View<const int *, HostSpace> h_s2f(s2f.data(), s2f.size());
  Kokkos::View<const short *, HostSpace> h_smask(
      &smask[0], smask.size());

  Kokkos::parallel_for(
      "VAR_side_vol", Kokkos::RangePolicy<HostExecSpace>(0, sl), [&](const int s) {
        if (h_smask(s) > 0) {
          Vec3 const &zc = h_zx(h_s2z(s));
          Vec3 const &p1 = h_px(h_s2p1(s));
          Vec3 const &p2 = h_px(h_s2p2(s));
          Vec3 const &fc = h_fx(h_s2f(s));
          /* Note that this is a signed volume of the tetrahedron formed by the
             zone center, face center, and edge endpoints. */
          auto const fz = fc - zc;
          auto const p1z = p1 - zc;
          auto const p2z = p2 - zc;
          auto const cp = crossprod(p2z, p1z);
          h_side_vol_k(s) = dotprod(fz, cp) / 6.0;
        } else
          h_side_vol_k(s) = 0.0;
      });

  sides().scatter(side_vol);
  VAR_INIT_EPILOGUE;
}

} // namespace SOA_Idx
} // namespace Ume
