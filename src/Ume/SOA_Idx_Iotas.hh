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

#ifndef UME_SOA_IDX_IOTAS_HH
#define UME_SOA_IDX_IOTAS_HH 1

#include "Ume/Entity_Field.hh"
#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {
//! SoA representation of mesh iotas
/*! An iota is a subzonal quantity that is a tet object that stores information
 * on a finer scale than a side. An iota consists of half of a side and is a
 * triangle/tetrahedron bounded by a set of points {z, f, p, e}. A side can be
 * considered to be a pair of tet objects.
*/
struct Iotas : public Entity {
  explicit Iotas(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Iotas const &rhs) const;
};

} // namespace SOA_Idx
} // namespace Ume
#endif
