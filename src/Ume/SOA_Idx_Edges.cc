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

/* --------------------------------- Edges ----------------------------------*/

Edges::Edges(Mesh *mesh) : Entity{mesh} {
  // map: edge to end point 1 index
  ds().insert("m:e>p1", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: edge to end point 2 index
  ds().insert("m:e>p2", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("ecoord", std::make_unique<VAR_ecoord>(*this));
}

void Edges::write(std::ostream &os) const {
  write_bin(os, std::string("edges"));
  Entity::write(os);
  IVWRITE("m:e>p1");
  IVWRITE("m:e>p2");
}

void Edges::read(std::istream &is) {
  std::string dummy;
  read_bin(is, dummy);
  assert(dummy == "edges");
  Entity::read(is);
  IVREAD("m:e>p1");
  IVREAD("m:e>p2");
}

bool Edges::operator==(Edges const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:e>p1") && EQOP("m:e>p2"));
}

void Edges::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:e>p1", total);
  RESIZE("m:e>p2", total);
}

bool Edges::VAR_ecoord::init_() const {
  VAR_INIT_PREAMBLE("VAR_ecoord");

  int const el = edges().local_size();
  int const ell = edges().size();
  auto const &e2p1{caccess_intv("m:e>p1")};
  auto const &e2p2{caccess_intv("m:e>p2")};
  auto const &pcoord{caccess_vec3v("pcoord")};
  auto const &emask{edges().mask};
  auto &ecoord = mydata_vec3v();
  ecoord.resize(ell);

  Kokkos::View<Vec3 *, HostSpace> h_ecoord(&ecoord[0], ecoord.size());
  Kokkos::View<const Vec3 *, HostSpace> h_pcoord(
      &pcoord[0], pcoord.size());
  Kokkos::View<const short *, HostSpace> h_emask(
      &emask[0], emask.size());
  Kokkos::View<const int *, HostSpace> h_e2p2(e2p2.data(), e2p2.size());
  Kokkos::View<const int *, HostSpace> h_e2p1(e2p1.data(), e2p1.size());

  Kokkos::parallel_for(
      "VAR_ecoord", Kokkos::RangePolicy<HostExecSpace>(0, el), [&](const int e) {
        if (h_emask(e)) {
          h_ecoord(e) = (h_pcoord(h_e2p1(e)) + h_pcoord(h_e2p2(e))) * 0.5;
        } else {
          h_ecoord(e) = 0.0;
        }
      });

  VAR_INIT_EPILOGUE;
}

} // namespace SOA_Idx
} // namespace Ume
