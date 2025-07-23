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

namespace Ume {
namespace SOA_Idx {

/* -------------------------------- Corners ---------------------------------*/

Corners::Corners(Mesh *mesh) : Entity{mesh} {
  // map: corner->index of characteristic point
  ds().insert("m:c>p", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // map: corner->index of parent zone zone
  ds().insert("m:c>z", std::make_unique<Ume::DS_Entry>(Types::INTV));
  ds().insert("corner_vol", std::make_unique<VAR_corner_vol>(*this));
  ds().insert("corner_csurf", std::make_unique<VAR_corner_csurf>(*this));
  ds().insert("m:c>s", std::make_unique<VAR_corner_to_sides>(*this));
}

void Corners::write(std::ostream &os) const {
  write_bin(os, std::string{"corners"});
  Entity::write(os);
  IVWRITE("m:c>p");
  IVWRITE("m:c>z");
}

void Corners::read(std::istream &is) {
  std::string dummy;
  read_bin(is, dummy);
  assert(dummy == "corners");
  Entity::read(is);
  IVREAD("m:c>p");
  IVREAD("m:c>z");
}

bool Corners::operator==(Corners const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:c>p") && EQOP("m:c>z"));
}

void Corners::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:c>p", total);
  RESIZE("m:c>z", total);
}

bool Corners::VAR_corner_vol::init_() const {
  VAR_INIT_PREAMBLE("VAR_corner_vol");

  int const cll = corners().size();
  int const sl = sides().local_size();
  auto const &s2c1{caccess_intv("m:s>c1")};
  auto const &s2c2{caccess_intv("m:s>c2")};
  auto const &side_vol{caccess_dblv("side_vol")};
  auto const &smask{sides().mask};
  auto &corner_vol = mydata_dblv();
  corner_vol.assign(cll, 0.0);

  for (int s = 0; s < sl; ++s) {
    if (smask[s] > 0) {
      double const hsv = 0.5 * side_vol[s];
      corner_vol[s2c1[s]] += hsv;
      corner_vol[s2c2[s]] += hsv;
    }
  }
  corners().scatter(corner_vol);
  VAR_INIT_EPILOGUE;
}

bool Corners::VAR_corner_csurf::init_() const {
  VAR_INIT_PREAMBLE("VAR_corner_csurf");

  int const cll = corners().size();
  int const sl = sides().local_size();
  auto const &s2c1{caccess_intv("m:s>c1")};
  auto const &s2c2{caccess_intv("m:s>c2")};
  auto const &side_surf{caccess_vec3v("side_surf")};
  auto const &smask{sides().mask};
  auto &corner_csurf = mydata_vec3v();
  corner_csurf.assign(cll, Vec3(0.0));

  for (int s = 0; s < sl; ++s) {
    if (smask[s]) {
      corner_csurf[s2c1[s]] += side_surf[s];
      corner_csurf[s2c2[s]] -= side_surf[s];
    }
  }
  VAR_INIT_EPILOGUE;
}

bool Corners::VAR_corner_to_sides::init_() const {
  VAR_INIT_PREAMBLE("VAR_corner_to_sides");

  int const cll = corners().size();
  int const sll = sides().size();
  auto const &s2c1{caccess_intv("m:s>c1")};
  auto const &s2c2{caccess_intv("m:s>c2")};
  auto &corner_to_sides = mydata_intrr();
  corner_to_sides.init(cll);

  std::vector<std::vector<int>> accum(cll);
  for (int s = 0; s < sll; ++s) {
    accum.at(s2c1[s]).push_back(s);
    accum.at(s2c2[s]).push_back(s);
  }

  for (int c = 0; c < cll; ++c) {
    corner_to_sides.assign(c, accum[c].begin(), accum[c].end());
  }
  VAR_INIT_EPILOGUE;
}

} // namespace SOA_Idx
} // namespace Ume
