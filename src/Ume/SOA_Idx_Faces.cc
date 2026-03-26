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
  \file Ume/SOA_Idx_Faces.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/soa_idx_helpers.hh"
#include "Ume/mem_exec_spaces.hh"
#include <cassert>

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Faces ----------------------------------*/

Faces::Faces(Mesh *mesh) : Entity{mesh} {
  // map: face to adjacent zone 1 index
  ds().insert("m:f>z1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: face to adjacent zone 2 index
  ds().insert("m:f>z2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("fcoord", std::make_unique<VAR_fcoord>(*this));
}

void Faces::write(std::ostream &os) const {
  write_bin(os, std::string("faces"));
  Entity::write(os);
  IVWRITE("m:f>z1");
  IVWRITE("m:f>z2");
}

void Faces::read(std::istream &is) {
  std::string dummy;
  read_bin(is, dummy);
  assert(dummy == "faces");
  Entity::read(is);
  IVREAD("m:f>z1");
  IVREAD("m:f>z2");
}

bool Faces::operator==(Faces const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:f>z1") && EQOP("m:f>z2"));
}

void Faces::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:f>z1", total);
  RESIZE("m:f>z2", total);
}

bool Faces::VAR_fcoord::init_() const {
  VAR_INIT_PREAMBLE("VAR_fcoord");

  int const fll{faces().size()};
  int const fl{faces().local_size()};
  int const sl{sides().local_size()};
  auto const &s2f{caccess_intv("m:s>f")};
  auto const &s2p1{caccess_intv("m:s>p1")};
  auto const &pcoord{caccess_vec3v("pcoord")};
  auto const &smask{sides().mask};
  auto &fcoord = mydata_vec3v();
  fcoord.resize(fll, Vec3(0.0));

  std::vector<int> num_face_pts(fl, 0);

  Kokkos::View<Vec3 *, HostSpace> h_fcoord(&fcoord[0], fcoord.size());
  Kokkos::View<const Vec3 *, HostSpace> h_pcoord(
      &pcoord[0], pcoord.size());
  Kokkos::View<const short *, HostSpace> h_smask(
      &smask[0], smask.size());
  Kokkos::View<const int *, HostSpace> h_s2f(s2f.data(), s2f.size());
  Kokkos::View<const int *, HostSpace> h_s2p1(s2p1.data(), s2p1.size());
  Kokkos::View<int *, HostSpace> h_num_face_pts(
      num_face_pts.data(), num_face_pts.size());

  Kokkos::parallel_for(
      "VAR_fcoord-1", Kokkos::RangePolicy<HostExecSpace>(0, sl), [&](const int s) {
        if (h_smask(s)) {
          int const f = h_s2f(s);
#if defined(UME_SERIAL)
          h_fcoord(f) += h_pcoord(h_s2p1(s));
#else
          Kokkos::atomic_add(&h_fcoord(f), h_pcoord(h_s2p1(s)));
#endif
          h_num_face_pts.access(f) += 1;
        }
      });
  auto const &fmask{faces().mask};

  Kokkos::View<const short *, HostSpace> h_fmask(
      &fmask[0], fmask.size());
  Kokkos::parallel_for(
      "VAR_fcoord-2", Kokkos::RangePolicy<HostExecSpace>(0, fl), [&](const int f) {
        if (h_fmask(f)) {
          h_fcoord(f) /= static_cast<double>(h_num_face_pts(f));
        }
      });

  VAR_INIT_EPILOGUE;
}
} // namespace SOA_Idx
} // namespace Ume
