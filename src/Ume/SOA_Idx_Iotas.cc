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
  \file Ume/SOA_Idx_Iotas.hh
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/soa_idx_helpers.hh"
#include <cassert>

namespace Ume {
namespace SOA_Idx {

Iotas::Iotas(Mesh *mesh) : Entity{mesh} {
  // Index of parent mesh zone
  ds().insert("m:a>z", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Index of the mesh face of z that contains e
  ds().insert("m:a>f", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Indices of the points of 'e' (one per 'a', two per 's')
  ds().insert("m:a>p", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Index of characteristic edge
  ds().insert("m:a>e", std::make_unique<Ume::DS_Entry>(Types::INTV));
  // Indices of the sides that each tet object is part of
  ds().insert("m:a>s", std::make_unique<Ume::DS_Entry>(Types::INTV));
}

void Iotas::write(std::ostream &os) const {
  write_bin(os, std::string{"iotas"});
  Entity::write(os);
  IVWRITE("m:a>z");
  IVWRITE("m:a>f");
  IVWRITE("m:a>p");
  IVWRITE("m:a>e");
  IVWRITE("m:a>s");
}

void Iotas::read(std::istream &is) {
  std::string dummy;
  read_bin(is, dummy);
  assert(dummy == "iotas");
  Entity::read(is);
  IVREAD("m:a>z");
  IVREAD("m:a>f");
  IVREAD("m:a>p");
  IVREAD("m:a>e");
  IVREAD("m:a>s");
}

bool Iotas::operator==(Iotas const &rhs) const {
  return (Entity::operator==(rhs) && EQOP("m:a>z") && EQOP("m:a>f") &&
      EQOP("m:a>p") && EQOP("m:a>e") && EQOP("m:a>s"));
}

void Iotas::resize(int const local, int const total, int const ghost) {
  Entity::resize(local, total, ghost);
  RESIZE("m:a>z", total);
  RESIZE("m:a>f", total);
  RESIZE("m:a>p", total);
  RESIZE("m:a>e", total);
  RESIZE("m:a>s", total);
}

} // namespace SOA_Idx
} // namespace Ume
